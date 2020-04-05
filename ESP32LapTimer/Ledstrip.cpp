#include "Ledstrip.h"

#include "ADC.h"
#include "HardwareConfig.h"
//#include <Adafruit_NeoPixel.h>
//#include "settings_eeprom.h"
#include <FastLED.h>

#define DATA_PIN 17
#define NUM_LEDS 12

#define COLOR_RX_1 255

//Adafruit_NeoPixel pixels(NUMPIXELS, PIXELS_PIN, NEO_GRB + NEO_KHZ400);
CRGB leds[NUM_LEDS];

void ledsSetup() {
    FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  //pixels.begin();
  fillColor(50, 50, 50); // low white
}

void setRaceModeLeds(bool raceMode) {
  if (raceMode) {
    fillColor(203, 20, 188); // Pinkish
  } else {
    fillColor(50, 50, 50); // low white
  }
}

void setLapLeds(uint8_t reciever) {
  switch (reciever) {
  case 0:
    fillColor(0, 0, 255); // Blue
    break;
  case 1:
    fillColor(230, 170, 0); // Yellow
    break;
  case 2:
    fillColor(203, 0, 0); // Red
    break;
  case 3:
    fillColor(0, 230, 0); // Green
    break;
  case 4:
    fillColor(20, 230, 230); // Turquise
    break;
  case 5:
    fillColor(203, 20, 188); // Pinkish
    break;
  }
}

void fillColor(uint8_t r, uint8_t g, uint8_t b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    //pixels.setPixelColor(i, pixels.Color(r, g, b));
    leds[i] = CRGB(r, g, b);
  }

  //noInterrupts();
  //pixels.show();
    FastLED.show();
  //interrupts();
}
