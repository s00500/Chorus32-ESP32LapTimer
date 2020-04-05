#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include <esp_task_wdt.h>

#include "ADC.h"
#include "Bluetooth.h"
#include "Comms.h"
#include "HardwareConfig.h"
#include "Ledstrip.h"
#include "RX5808.h"
#include "settings_eeprom.h"
#ifdef OLED
#include "OLED.h"
#endif
#include "Beeper.h"
#include "Calibration.h"
#include "Output.h"
#include "WebServer.h"
#ifdef USE_BUTTONS
#include "Buttons.h"
#endif
#include "Laptime.h"
#include "Utils.h"
#include "Watchdog.h"
#include "WebServer.h"

//#define BluetoothEnabled //uncomment this to use bluetooth (experimental, ble + wifi appears to cause issues)

#define MAX_SRV_CLIENTS 5
WiFiClient serverClients[MAX_SRV_CLIENTS];

static TaskHandle_t adc_task_handle = NULL;

void IRAM_ATTR adc_read()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  /* un-block the interrupt processing task now */
  vTaskNotifyGiveFromISR(adc_task_handle, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken)
  {
    portYIELD_FROM_ISR();
  }
}

void IRAM_ATTR adc_task(void *args)
{
  watchdog_add_task();
  while (42)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    nbADCread(NULL);
    watchdog_feed();
  }
}

void setup()
{

#ifdef OLED
  oledSetup();
#endif

  Serial.begin(115200);
  Serial.println("Booting....");
#ifdef USE_BUTTONS
  newButtonSetup();
#endif

  EepromSettings.setup();

  delay(500);
  InitHardwarePins();
  ConfigureADC();

  ledsSetup();

  InitSPI();
  // PowerDownAll(); // Powers down all RX5808's
  delay(250);

  // try to connect to existing network
  WiFi.begin("SSID", "PASSWORD");
  Serial.print("\n\nTry to connect to existing network (Bachinet)");

  uint8_t timeout = 10;
  // Wait for connection, 5s timeout
  do
  {
    delay(500);
    Serial.print(".");
    timeout--;
  } while (timeout && WiFi.status() != WL_CONNECTED);

  // not connected -> create hotspot
  if (WiFi.status() != WL_CONNECTED)
  {
    // try to connect to existing network
    WiFi.begin("BachiMobilWolke", "bachis987");
    Serial.print("\n\nTry to connect to existing network (BachiMobilWolke)");

    // Serial.print("\n\nCreating hotspot");

    // InitWifiAP();
  }

  InitWebServer();

  if (!EepromSettings.SanityCheck())
  {
    EepromSettings.defaults();
    Serial.println("Detected That EEPROM corruption has occured.... \n Resetting EEPROM to Defaults....");
  }

  setRXADCfilter(EepromSettings.RXADCfilter);
  setADCVBATmode(EepromSettings.ADCVBATmode);
  setVbatCal(EepromSettings.VBATcalibration);
  commsSetup();
  init_outputs();

  for (int i = 0; i < getNumReceivers(); i++)
  {
    setRSSIThreshold(i, EepromSettings.RSSIthresholds[i]);
  }

  xTaskCreatePinnedToCore(adc_task, "ADCreader", 4096, NULL, 1, &adc_task_handle, 0);
  hw_timer_t *adc_task_timer = timerBegin(0, 8, true);
  timerAttachInterrupt(adc_task_timer, &adc_read, true);
  timerAlarmWrite(adc_task_timer, 1667, true); // 6khz -> 1khz per adc channel
  timerAlarmEnable(adc_task_timer);

  // SelectivePowerUp();

  // inits modules with defaults.  Loops 10 times  because some Rx modules dont initiate correctly.
  for (int i = 0; i < getNumReceivers() * 10; i++)
  {
    setModuleChannelBand(i % getNumReceivers());
  }

  // beep();
}

void loop()
{
  rssiCalibrationUpdate();
  // touchMonitor(); // A function to monitor capacitive touch values, defined in buttons.ino

  //  if (shouldReboot) {  //checks if reboot is needed
  //    Serial.println("Rebooting...");
  //    delay(100);
  //    ESP.restart();
  //  }
#ifdef USE_BUTTONS
  newButtonUpdate();
#endif
#ifdef OLED
  OLED_CheckIfUpdateReq();
#endif
  sendNewLaps();
  update_outputs();
  SendCurrRSSIloop();
  updateWifi();

  EepromSettings.save();
  beeperUpdate();
  if (UNLIKELY(!isInRaceMode()))
  {
    thresholdModeStep();
  }
}
