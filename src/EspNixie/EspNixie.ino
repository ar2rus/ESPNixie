/**
 * Use 2.6.3 esp8266 core (3.0+ -> do not work)
 * lwip v2 Higher bandwidth; CPU 160 MHz
 * 1M (128K)
 * 
 * to migrate to 2.7.0+ see config time changes: https://github.com/esp8266/Arduino/issues/7353
 * 
 * OneWire 2.3.0
 */

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
 
#include <time.h>
#include <TimeLib.h>

#include "Credentials.h"
#include "Tasks.h"
#include "Narodmon.h"
#include "HistoryObservation.h"
#include "InsideTermometer.h"
#include "Nixie.h"
#include "Leds.h"

#define ALARM_DURATION 15000
#define NUMBER_DURATION 5000
#define STOPWATCH_DURATION 5000

#define SYSTEM_LEDS_BRIGHTNESS 50

IPAddress ip(192, 168, 1, 130); //Node static IP
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dnsAddr(192, 168, 1, 1);

#define TASK_UPDATE_PERIOD 50

TaskWrapper* tw = new TaskWrapper(TASK_UPDATE_PERIOD);
Narodmon* nm = new Narodmon(WiFi.macAddress());
Leds* leds = new Leds();
HistoryObservation* historyObserver = new HistoryObservation();

AsyncWebServer server(80);




//хранит параметр, переданный через метод /number
//его передача в callContinuousTask возможна только
//через глобальную переменную
char number_to_show[NIXIE_DIGITS_COUNT];

void config_time(float timezone_hours_offset, int daylightOffset_sec,
                 const char* server1, const char* server2, const char* server3) {
  configTime((int)(timezone_hours_offset * 3600), daylightOffset_sec, server1, server2, server3);
  INFO("Timezone: %f; daylightOffset: %d", timezone_hours_offset, daylightOffset_sec);
}

void config_narodmon(String apiKey, uint8_t use_latlng, double lat, double lng, uint8_t radius) {
  nm->setApiKey(apiKey);
  if (use_latlng) {
    nm->setConfigLatLng(lat, lng);
  } else {
    nm->setConfigUseLatLng(0);
  }
  nm->setConfigRadius(radius);
}

void show_none() {
  nixie_clear();
  leds->backlight();
}

const CRGB MINUS_COLOR = CRGB::Blue;

void show_value(float v, uint8_t pos_1, int num_frac){
  nixie_set(v, pos_1, num_frac);

  if (v < 0) {  ///minus value
    leds->set([&](CRGB* leds, uint8_t leds_num){  //используем вариант с _backlight_brightness
 
        int lb = pos_1;
        float t = abs(v);
        do{
          lb++;
        }while (((int)(t = t/10)) > 0);
    
        lb = _min(lb, leds_num);
        int rb = _max(pos_1 - num_frac, 0);
        for (int i=rb; i<lb; i++){
          leds[i] = MINUS_COLOR;
        }
    });
  }else{
    leds->backlight();
  }
}

uint8_t available_clock() {
  //если хоть раз синхронизировались, то отлично от 0
  time_t t;
  time(&t);
  return t;
}

void _clock(){
  time_t t;
  time(&t);
  if (t){
    int h = hour(t);
    int m = minute(t);
    int s = second(t);
    char p = t % 2;
    nixie_set(digit_code(h / 10, 1, 0), digit_code(h % 10, 1, p), digit_code(m / 10, 1, 0), digit_code(m % 10, 1, p), digit_code(s / 10, 1, 0), digit_code(s % 10, 1, p));
  }else{
    nixie_clear();
  }
}

void show_clock() {
  _clock();
  leds->backlight();
}

uint8_t available_t() {
  return nm->hasT();
}

uint32_t stopwatch_start_time = 0;
uint32_t stopwatch_pause_time = 0;

void _stopwatch(){
  if (stopwatch_start_time && !stopwatch_pause_time){
    uint32_t st = millis() -  stopwatch_start_time;
    int ms = (st % 1000) / 100;
    int s = (st / 1000) % 60;
    int m = (st / 60000);
    int p = s % 2;
    nixie_set(digit_code(m / 10, 1, 0), digit_code(m % 10, 1, p), digit_code(s / 10, 1, 0), digit_code(s % 10, 1, p), digit_code(ms, 1, 0), digit_code(0, 0, 0));
  }
}

void show_stopwatch() {
  _stopwatch();
  leds->backlight();
}

void _stopwatch_start(){
  stopwatch_start_time = millis();
  stopwatch_pause_time = 0;
}

void _stopwatch_stop(){
  stopwatch_start_time = 0;
}

void _stopwatch_toggle_pause(){
  if (available_stopwatch()){
    uint32_t t = millis();
    if (stopwatch_pause_time){
      stopwatch_start_time += (t - stopwatch_pause_time);
      stopwatch_pause_time = 0;
    }else{
      stopwatch_pause_time = t;
    }
  }
}

uint8_t available_stopwatch() {
  return stopwatch_start_time;
}

void show_t() {
  show_value(nm->getT(), 2, 1);
}

uint8_t available_p() {
  return nm->hasP();
}

void show_p() {
  show_value(nm->getP(), 2, 1);
}

uint8_t available_h() {
  return nm->hasH();
}

void show_h() {
  show_value(nm->getH(), 2, 0);
}

uint8_t available_t_inside() {
  return insideTermometerHasValue();
}

void show_t_inside() {
  show_value(insideTermometerValue(), 2, 1);
}

void config_modes(uint32_t clock_duration, uint32_t t_duration, uint32_t p_duration, uint32_t h_duration, uint32_t t_inside_duration) {
  tw->reset();
  tw->addContinuousTask(clock_duration, available_clock, show_clock);
  tw->addContinuousTask(t_duration, available_t, show_t);
  
  if (p_duration){
    tw->addContinuousTask(clock_duration, available_clock, show_clock);
    tw->addContinuousTask(p_duration, available_p, show_p);
  }
  
  if (h_duration){
    tw->addContinuousTask(clock_duration, available_clock, show_clock);
    tw->addContinuousTask(h_duration, available_h, show_h);
  }
  
  if (t_inside_duration){
    tw->addContinuousTask(clock_duration, available_clock, show_clock);
    tw->addContinuousTask(t_inside_duration, available_t_inside, show_t_inside);
  }
  
  tw->addPeriodicalTask(1000, 60000, []() {
    insideTermometerRequest();
  });

  tw->addPeriodicalTask(10000, 150000, []() {
    nm->request();
  });

  tw->addPeriodicalTask(15000, 300000, []() {
    if (nm->hasP()){
      time_t t;
      time(&t);
      historyObserver->addNextValue(t, nm->getP()*10);
    }
  });
}

const CRGB PROGRESS_COLOR = CRGB::Blue;

uint8_t point_pos = 0;
void show_connecting_progress(){
   leds->set([&](CRGB* leds, uint8_t num_leds, uint8_t* brightness){
          
      for (int i = 0; i < num_leds; i++) {
        leds[i] = point_pos == i ? PROGRESS_COLOR : CRGB::Black;
      }
      if (point_pos++ >= num_leds){
        point_pos = 0;
      }

      *brightness = SYSTEM_LEDS_BRIGHTNESS;
   });
}

uint8_t getIntArg(AsyncWebServerRequest *request, String argName, int* argValue){
  if (request->hasArg(argName.c_str())){
      String v = request->arg(argName.c_str());
      
      char all_digits = 1;
      for (byte i = 0; i < v.length(); i++) {
        if (!isDigit(v.charAt(i))) {
            all_digits = 0;
            break;
         }
      }

      if(all_digits){
        *argValue = v.toInt();
        return 1;
      }
  }
  return 0;
}

uint8_t ota_progress;

void setup() {
  DEBUG("Booting");
  nixie_init();

  WiFi.mode(WIFI_STA);

  WiFi.begin(AP_SSID, AP_PASSWORD);
  WiFi.config(ip, gateway, subnet, dnsAddr);

  tw->addPeriodicalTask(0, 200, []() {
    show_connecting_progress();
  });
  
  while (WiFi.status() != WL_CONNECTED) {
    tw->update();
    delay(100);
  }
  
  INFO("IP address: %s", WiFi.localIP().toString().c_str());

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/narodmon_r", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", Logging._response);
  });

server.on("/narodmon_e", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", Logging._error);
  });


  server.on("/error", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", Logging._error);
  });

  server.on("/time", HTTP_GET, [](AsyncWebServerRequest * request) {
    time_t t;
    time(&t);
    request->send(200, "text/plain", String(t));
  });

  server.on("/history", HTTP_GET, [](AsyncWebServerRequest * request) {
    time_t t;
    time(&t);

    if(request->hasArg("hour")){
       int h;
       if (getIntArg(request, "hour", &h)){
          request->send(200, "text/plain", String(t) + ": " + String(historyObserver->getHourValue(t - h * 3600)));
       }
    }else{
      request->send(200, "text/plain", String(t) + ": " + String(historyObserver->getDayValue(t)));
    }
  });

  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
     tw->callContinuousTask([]() {
        ESP.restart();
     });
    
    show_none();
    request->send(200);
  });

  server.on("/clock", HTTP_GET, [](AsyncWebServerRequest * request) {
    tw->callContinuousTask(10000, available_clock, show_clock);
    request->send(200);
  });

  server.on("/t", HTTP_GET, [](AsyncWebServerRequest * request) {
    tw->callContinuousTask(10000, available_t, show_t);
    request->send(200);
  });

  server.on("/p", HTTP_GET, [](AsyncWebServerRequest * request) {
    tw->callContinuousTask(10000, available_p, show_p);
    request->send(200);
  });

  server.on("/h", HTTP_GET, [](AsyncWebServerRequest * request) {
    tw->callContinuousTask(10000, available_h, show_h);
    request->send(200);
  });

  server.on("/i", HTTP_GET, [](AsyncWebServerRequest * request) {
    tw->callContinuousTask(10000, available_t_inside, show_t_inside);
    request->send(200);
  });
  
  server.on("/t_value", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", nm->hasT() ? String(nm->getT(), 1) : "");
  });

  server.on("/p_value", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", nm->hasP() ? String(nm->getP(), 1) : "");
  });

  server.on("/h_value", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", nm->hasH() ? String(nm->getH(), 1) : "");
  });

  server.on("/i_value", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", insideTermometerHasValue() ? String(insideTermometerValue(), 1) : "");
  });

  server.on("/alarm", HTTP_GET, [](AsyncWebServerRequest * request) {
    tw->callContinuousTask(ALARM_DURATION, NULL, []() {
      _clock();
      leds->rainbow();
    });
    
    request->send(200);
  });

  server.on("/tz", HTTP_GET, [](AsyncWebServerRequest * request) {
    config_time(request->arg("tz").toInt(), 0, "pool.ntp.org", "time.nist.gov", NULL);
    request->send(200);
  });

   server.on("/stopwatch", HTTP_GET, [](AsyncWebServerRequest *request) {
      int status_code = 404;
      if (request->params() == 0 || (request->params() == 1 && request->hasArg("start"))){
        _stopwatch_start();
        tw->callContinuousTask(available_stopwatch, show_stopwatch);
        status_code = 200;
      }else if (request->params() == 1){
        if (request->hasArg("stop")){
          if (available_stopwatch()){
            _stopwatch_stop();
            tw->callContinuousTask(STOPWATCH_DURATION, show_stopwatch);
            status_code = 200;
          }
        }else if (request->hasArg("pause")){
          if (available_stopwatch()){
            _stopwatch_toggle_pause();
            tw->callContinuousTask(available_stopwatch, show_stopwatch);
            status_code = 200;
          }
        }
      }
      request->send(status_code);
   });

  server.on("/number", HTTP_GET, [](AsyncWebServerRequest *request) {
    int r = 404;
    if(request->hasArg("v")){ //отображает последние(младшие) NIXIE_DIGITS_COUNT(6) цифр переданного номера
      String v = request->arg("v");
      int length = _min(v.length(), NIXIE_DIGITS_COUNT);
      v = v.substring(_max(v.length() - length, 0));

      char all_digits = 1;
      for (byte i = 0; i < v.length(); i++) {
        if (!isDigit(v.charAt(i))) {
            all_digits = 0;
            break;
         }
      }
      
      if (all_digits) {
         nixie_int(v.toInt(), 0, length, number_to_show);
         tw->callContinuousTask(NUMBER_DURATION, NULL, []() {
            nixie_set(number_to_show);
         });
        r = 200;
      }
      
    }
    request->send(r);
  });


  #define REST_LED "/led"
  #define REST_LED_PARAM_BRIGHTNESS "brightness"
  #define REST_LED_PARAM_UP "up"
  #define REST_LED_PARAM_DOWN "down"
  
  server.on(REST_LED, HTTP_GET, [&](AsyncWebServerRequest* request) {
    int status_code = 200;
    if (request->params() == 0){
      leds->backlight_toggle();
    }else{
      if (request->hasParam(REST_LED_PARAM_BRIGHTNESS)){
        int brightness;
        if(getIntArg(request, REST_LED_PARAM_BRIGHTNESS, &brightness)){
          if (brightness <= 255){
            leds->setBacklightBrightness(brightness);
          }else{
            status_code = 404;
          }
        }else{
          status_code = 404;
        }
      } else if (request->hasParam(REST_LED_PARAM_UP)){
        leds->upBacklightBrightness();
      } else if (request->hasParam(REST_LED_PARAM_DOWN)){
        leds->downBacklightBrightness();
      } else {
        status_code = 404;
      }

      if (status_code == 200){
        leds->backlight_on();
      }
    }
    request->send(status_code);
  });

  server.onNotFound( [](AsyncWebServerRequest * request) {
    request->send(404, "text/plain", "File Not Found\n\n");
  });
  server.begin();


  ArduinoOTA.setHostname("esp-nixie");
  ArduinoOTA.onStart([]() {
    //if (ArduinoOTA.getCommand() == U_FLASH)
    //  type = "sketch";
    //else // U_SPIFFS
    //  type = "filesystem";
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()

    ota_progress = 0;
    tw->callContinuousTask([]() {
      
      nixie_set(ota_progress, 0);
      leds->backlight_off();
      
      const CRGB OTA_PROGRESS_COLOR = CRGB::Orange;
      
      leds->set([&](CRGB* leds, uint8_t num_leds, uint8_t* brightness){
        for (int i = 0; i < num_leds; i++) {
          if (ota_progress >= (i + 1) * 100 / num_leds) {
            leds[i] = OTA_PROGRESS_COLOR;
          } else {
            break;
          }
        }
        *brightness = SYSTEM_LEDS_BRIGHTNESS;
      });
    });
  });

  ArduinoOTA.onEnd([]() {
    leds->backlight_off();
    nixie_clear_force();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    ota_progress = (progress / (total / 100));
    tw->update(); //main loop does not call while OTA
  });


  ArduinoOTA.begin();

  insideTermometerInit();
  config_time(4, 0, "pool.ntp.org", "time.nist.gov", NULL);
  config_narodmon("9M5UhuQA2c8f8", 1, 53.2266, 50.1915, 5);
  config_modes(7000, 3000, 3000, 0, 0);
  
  INFO("Setup done");
}

void loop() {
  tw->update();
  
  ArduinoOTA.handle();
  yield();
}
