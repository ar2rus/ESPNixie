#include "Leds.h"


Leds::Leds(){
	FastLED.addLeds<WS2812B, LED_PIN, GRB>(_leds, _num).setCorrection(TypicalPixelString);
	backlight();
}

void Leds::set(LedsBrightnessSetupHandlerFunction setupFunction){
	CRGB leds[_num];
  get(leds);
  
	uint8_t brightness = FastLED.getBrightness();
	
  if (setupFunction){
    setupFunction(leds, _num, &brightness);
  }
	
	bool changed = false;
	for (int i = 0; i < _num; i++) {
		if (_leds[i] != leds[_num-i-1]) {
			changed = true;
			_leds[i] = leds[_num-i-1];
		}
	}

	changed |= (brightness != FastLED.getBrightness());

	if (changed) {
		FastLED.setBrightness(brightness);
		FastLED.show();
	}
}

void Leds::set(LedsSetupHandlerFunction setupFunction){
  if (setupFunction){
    set([&](CRGB* leds, uint8_t num_leds, uint8_t* brightness){
      setupFunction(leds, num_leds);
      *brightness = _backlight_brightness;
    });
  }
}

void Leds::set(CRGB color, uint8_t _brightness){
	set([&](CRGB* leds, uint8_t leds_num, uint8_t* brightness){
		fill_solid(leds, leds_num, color);
    *brightness = _brightness;
	});
}

uint8_t Leds::get(CRGB* leds){
  for (int i=0; i<_num; i++){
    leds[_num-i-1] = _leds[i];
  }
  return _num;
}

String Leds::info(){
    String value = "";
    for (int i=_num-1; i<0; i--){
      String r = String(_leds[i].r, HEX);
      if (r.length() < 2){
        r = "0" + r;
      }
      String g = String(_leds[i].g, HEX);
      if (g.length() < 2){
        g = "0" + g;
      }
      String b = String(_leds[i].b, HEX);
      if (b.length() < 2){
        b = "0" + b;
      }
      value += (r + g + b) + "\r\n";
    }
    return value;
}

void Leds::backlight(){
  set(_backlight_on ? _backlight_color : CRGB::Black, _backlight_brightness);
}

#define BPM       60
#define DIMMEST   0
#define BRIGHTEST 255

static uint16_t hue16 = 0;
		  
void Leds::rainbow(){
	  set([](CRGB* leds, uint8_t leds_num, uint8_t* brightness){
		  hue16 += 9;
		  fill_rainbow( leds, leds_num, hue16 / 256, 0);
		  *brightness = beatsin8( BPM, DIMMEST, BRIGHTEST);
	  });
}

void Leds::backlight_on(bool on){
	_backlight_on = on;
 backlight();
}

void Leds::backlight_on(){
	backlight_on(true);
}

void Leds::backlight_off(){
	backlight_on(false);
}

void Leds::backlight_toggle(){
	backlight_on(!_backlight_on);
}

void Leds::setBacklightBrightness(uint8_t brightness){
  if (brightness>=BRIGHTNESS_MIN_VALUE && brightness<BRIGHTNESS_MAX_VALUE){
    _backlight_brightness = brightness;
  }
}

void Leds::setBacklightColor(CRGB color){
	_backlight_color = color;
}

void Leds::upBacklightBrightness(uint8_t delta){
  int nval = _backlight_brightness + delta;
  nval = min(nval, BRIGHTNESS_MAX_VALUE);
  setBacklightBrightness(nval);
}
  
void Leds::downBacklightBrightness(uint8_t delta){
  int nval = _backlight_brightness - delta;
  nval = max(nval, BRIGHTNESS_MIN_VALUE);
  setBacklightBrightness(nval);
}
