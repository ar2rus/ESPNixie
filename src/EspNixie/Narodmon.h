#ifndef NARODMON_h
#define NARODMON_h

#include <stdint.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "ESPAsyncTCP.h"

#include "JsonStreamingParser.h"
#include "JsonListener.h"

#define NARODMON_HOST "narodmon.ru"
#define NARODMON_PORT 80
#define NARODMON_API_PATH "/api"

#define NARODMON_TYPE_TEMPERATURE 1
#define NARODMON_TYPE_HUMIDITY 2
#define NARODMON_TYPE_PRESSURE 3

//#define NARODMON_TIMEOUT 5

#define JSON_BUFFER_SIZE 300

//минимальная задержка между запросами
#define MIN_REQUEST_PERIOD 1*60*1000

//время ожидания ответа
#define RESPONSE_TIMEOUT 2 * 1000

//максимальное время использования полученного
//значения температуры
#define T_MAX_TIME 30*60*1000

//максимальное время использования полученного
//значения влажности
#define H_MAX_TIME 10*60*1000

//максимальное время использования полученного
//значения давления
#define P_MAX_TIME 1*60*60*1000

#define REQUEST_DEVICES_LIMIT 20
#define SENSOR_COUNT_LIMIT REQUEST_DEVICES_LIMIT * 3

#define VALUE_NONE INT16_MAX

enum RESOLVE_MODE {CLOSEST, MIN, MAX, AVG};

struct sensor_value {
  int8_t type;
  int16_t value;
  int16_t distance;
};

struct device {
  int16_t distance;
};

class Narodmon: public JsonListener{
private:
    String config_uuid;
    String config_apiKey;
    
    uint8_t config_useLatLng;
    double config_lat;
    double config_lng;
    
    uint8_t config_radius = 0;

    uint8_t config_reqT = 1;
    uint8_t config_reqH = 1;
    uint8_t config_reqP = 1;
    
    //время получения последнего значения температуры
    uint32_t t_time = 0;
    //последнее полученное значение температуры
    int16_t t;

    //время получения последнего значения отн.влажности
    uint32_t h_time = 0;
    //последнее полученное значение отн.влажности
    int16_t h;
    
    //время получения последнего значения давления
    uint32_t p_time = 0;
    //последнее полученное значение давления
    int16_t p;

    //время инициализации последнего запроса
    uint32_t request_time = 0;
    //флаг ожидания/получения ответа
    //volatile uint8_t waiting_response = 0;
    
    AsyncClient * aClient = NULL;

    
    JsonStreamingParser parser;
    String parser_key;

    uint8_t values_cnt;
    sensor_value values[SENSOR_COUNT_LIMIT];

    device d;
    
public:   
    Narodmon(String device_id);

    void setConfigUseLatLng(uint8_t use);
    void setConfigLatLng(double lat, double lng);
    void setConfigRadius(uint8_t radius);
    void setApiKey(String api_key);

    void setConfigReqT(uint8_t reqT);
    void setConfigReqH(uint8_t reqH);
    void setConfigReqP(uint8_t reqP);
    
    uint8_t request();
    
    uint8_t hasT();
    float getT();

    uint8_t hasH();
    float getH();
    
    uint8_t hasP();
    float getP();

    virtual void whitespace(char c);
    virtual void startDocument();
    virtual void key(String key);
    virtual void value(String value);
    virtual void endArray();
    virtual void endObject();
    virtual void endDocument();
    virtual void startArray();
    virtual void startObject();

};

enum LoggingLevel {
  LoggingLevelDebug,
  LoggingLevelInfo,
  LoggingLevelError,
};

class LoggingClass {
  private:
    boolean _enabled;
  public:
    LoggingClass(boolean enabled=false) {
      _enabled = enabled;
    };
  
    String _response;
    String _error;
    
    void log(LoggingLevel level, const char *fmt, ...);
    
    void reset(){
      _response = "";
     };
};
extern class LoggingClass Logging;

#define DEBUG(...) Logging.log(LoggingLevelDebug, __VA_ARGS__)
#define ERROR(...) Logging.log(LoggingLevelError, __VA_ARGS__)
#define INFO(...)  Logging.log(LoggingLevelInfo, __VA_ARGS__)

#define RESET(...) Logging.reset();

#endif
