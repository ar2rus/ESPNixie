#ifndef LEDS_h
#define LEDS_h

#define FASTLED_INTERNAL
#define FASTLED_ALLOW_INTERRUPTS 0
//#define FASTLED_INTERRUPT_RETRY_COUNT 10
#include <FastLED.h>

#include <functional>

#define LED_PIN 5
#define LED_COUNT 6

const CRGB DEFAULT_BACKLIGHT_COLOR = CRGB::White;
#define DEFAULT_BACKLIGHT_BRIGHTNESS 40

#define BRIGHTNESS_MIN_VALUE 10
#define BRIGHTNESS_MAX_VALUE 255

typedef std::function<void(CRGB* leds, uint8_t leds_num, uint8_t* brightness)> LedsBrightnessSetupHandlerFunction;
typedef std::function<void(CRGB* leds, uint8_t leds_num)> LedsSetupHandlerFunction;


class Leds{
private:
  const uint8_t _num = LED_COUNT;
  
	CRGB _leds[LED_COUNT];
	
	bool _backlight_on = false;
  CRGB _backlight_color = DEFAULT_BACKLIGHT_COLOR;
	uint8_t _backlight_brightness = DEFAULT_BACKLIGHT_BRIGHTNESS;
   
	void backlight_on(bool on);
public:
  Leds();

  uint8_t get(CRGB* leds);
  String info();
	
	void backlight();		//switches to backlight (if enabled)
	void set(CRGB color, uint8_t brightness);	//shows all leds with the same color
	void set(LedsSetupHandlerFunction setupFunction);	//show custom colors with backlight brightness level
  void set(LedsBrightnessSetupHandlerFunction setupFunction); //show custom colors & brightness
	void rainbow();			//shows rainbow
	
	void backlight_on();
	void backlight_off();
	void backlight_toggle();

	void setBacklightBrightness(uint8_t brightness);
	void setBacklightColor(CRGB color);

  void upBacklightBrightness(uint8_t delta = 10);
  void downBacklightBrightness(uint8_t delta = 10);
};

#endif
