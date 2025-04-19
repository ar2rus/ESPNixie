#include "Narodmon.h"

#include <ArduinoJson.h>
#include "JsonListener.h"

#include <MD5Builder.h>
#include <Arduino.h>

//#include "logging/Logging.h"

// Instantiate the shared instance that will be used.
LoggingClass Logging;

void LoggingClass::log(LoggingLevel level, const char *fmt, ...){
  if (!_enabled){
    return;
  }

  //static const char *logLevelPrefixes[] = { "D", "I", "E"};

  //_stream.print(logLevelPrefixes[level]);
  //_stream.print(" ");

  String* s = NULL;
  switch (level){
    case LoggingLevelError:
      s = &_error;
      break;
    default:
      s = &_response;
      break;
  }

  if (s){
    *s += String(millis());
    *s += ": ";
    va_list fmtargs;
    va_start(fmtargs, fmt);
    char tmp[512];
    vsnprintf(tmp, sizeof(tmp), fmt, fmtargs);
    va_end(fmtargs);
    *s += tmp;
    *s += "\r\n";
  }
}


Narodmon::Narodmon(String device_id){
  MD5Builder md5;
  md5.begin();
  md5.add(device_id);
  md5.calculate();
  config_uuid = md5.toString();

  parser.setListener(this);
}

void Narodmon::setApiKey(String api_key){
  DEBUG("setApiKey: %s", api_key.c_str());
  config_apiKey = api_key;
}

void Narodmon::setConfigUseLatLng(uint8_t use){
  DEBUG("setConfigUseLatLng: %u", use);
  config_useLatLng = use;
}

void Narodmon::setConfigLatLng(double lat, double lng){
  DEBUG("setConfigLatLng: %f; %f", lat, lng);
  config_lat = lat;
  config_lng = lng;
  setConfigUseLatLng(1);
}

void Narodmon::setConfigRadius(uint8_t radius){
  DEBUG("setConfigRadius: %u", radius);
  config_radius = radius;
}

void Narodmon::setConfigReqT(uint8_t reqT){
  DEBUG("setConfigReqH: %u", reqT);
  config_reqT = reqT;
}

void Narodmon::setConfigReqH(uint8_t reqH){
  DEBUG("setConfigReqT: %u", reqH);
  config_reqH = reqH;
}

void Narodmon::setConfigReqP(uint8_t reqP){
  DEBUG("setConfigReqP: %u", reqP);
  config_reqP = reqP;
}
    
uint8_t Narodmon::request(){
  if (!aClient){
    RESET();
    DEBUG("New request");
   
    uint32_t tmp_t = millis();
    uint32_t delta_t = tmp_t - request_time;
    if ((request_time == 0) || (delta_t >= MIN_REQUEST_PERIOD)){

      aClient = new AsyncClient();
      if(!aClient){//could not allocate client
         ERROR("Couldn't allocate memory");
         return 0;
      }
      
      aClient->onError([this](void * arg, AsyncClient * client, int error){
        ERROR("Error %s (%i)", client->errorToString(error), error);
        
        //aClient = NULL;
        //delete client;
      }, NULL);

      aClient->onTimeout([this](void * arg, AsyncClient *client, uint32_t time) {
          ERROR("Timeout on client in %i", time);
          client->close();
      }, NULL);

      aClient->onDisconnect([this](void * arg, AsyncClient * client){
           DEBUG("Disconnected");
           
           aClient = NULL;
           delete client;
       }, NULL);

      aClient->onData([this](void * arg, AsyncClient * client, void * data, size_t len){
            DEBUG ("Got data: %i bytes", len);
            
            char * d = (char*)data;
            for(size_t i=0; i<len;i++){
              parser.parse(d[i]);
            }
      }, NULL);

      aClient->onConnect([this, tmp_t](void * arg, AsyncClient * client){
        DEBUG("Connected");
  
        DynamicJsonDocument request(JSON_BUFFER_SIZE);
        
        request["cmd"] = "sensorsNearby";
        if (config_useLatLng){
          request["lat"] = config_lat;
          request["lng"] = config_lng;
        }
        if (config_radius > 0){
          request["radius"] = config_radius;
        }
        JsonArray requestTypes = request.createNestedArray("types");
        if (config_reqT){
          requestTypes.add(NARODMON_TYPE_TEMPERATURE); //temperature
        }
        if (config_reqH){
          requestTypes.add(NARODMON_TYPE_HUMIDITY); //humidity
        }
        if (config_reqP){
          requestTypes.add(NARODMON_TYPE_PRESSURE); //pressure
        }
  
        request["limit"] = REQUEST_DEVICES_LIMIT;
        request["pub"] = 1;
        request["uuid"] = config_uuid;
        request["api_key"] = config_apiKey;
  
        String jsonRequest;
        serializeJson(request, jsonRequest);
        
        DEBUG("Request[%u]: %s", jsonRequest.length(), jsonRequest.c_str());

        values_cnt = 0;
        parser.reset();
        
        //send a request
        
        client->write("POST "); client->write(NARODMON_API_PATH); client->write(" HTTP/1.1\r\n");
        client->write("Host: "); client->write(NARODMON_HOST); client->write("\r\n");
        client->write("Content-Type: application/json\r\n");
        client->write("Cache-Control: no-cache\r\n");
        client->write("User-Agent: esp-nixie\r\n");
        client->write("Content-Length: "); client->write(String(jsonRequest.length()).c_str()); client->write("\r\n");
        client->write("\r\n");
        client->write(jsonRequest.c_str(), jsonRequest.length());

        request_time = tmp_t;

        aClient->onPoll([this](void * arg, AsyncClient * client){
            if (millis() - request_time > RESPONSE_TIMEOUT){
              ERROR("Poll timeout");
              client->close();
            }
        },NULL);
        
      }, NULL);

      if(!aClient->connect(NARODMON_HOST, NARODMON_PORT)){
        DEBUG("Connect fail");
        
        AsyncClient * client = aClient;
        aClient = NULL;
        delete client;
      }
    }else{
      DEBUG("Too short interval between requests. %u less then %u", delta_t, MIN_REQUEST_PERIOD);
    }
  }else{
    //DEBUG("Client is still working");
  }
  return 0;
}

uint8_t Narodmon::hasT(){
  return t_time > 0 && (millis() - t_time) < T_MAX_TIME;
}

float Narodmon::getT(){
  return t / 10.0;
}

uint8_t Narodmon::hasH(){
  return h_time > 0 && (millis() - h_time) < H_MAX_TIME;
}

float Narodmon::getH(){
  return h / 10.0;
}

uint8_t Narodmon::hasP(){
  return p_time > 0 && (millis() - p_time) < P_MAX_TIME;
}

float Narodmon::getP(){
  return p / 10.0;
}

void Narodmon::whitespace(char c) {
}

void Narodmon::startDocument() {
}

void Narodmon::key(String key) {
  parser_key = key;
}

void Narodmon::value(String value) {
  DEBUG("%s: %s", parser_key.c_str(), value.c_str());
  if (parser_key.equals("value")){
    values[values_cnt].value = (int)(value.toFloat()*10);
  }else if (parser_key.equals("type")){
    values[values_cnt].type = value.toInt();
  }else if (parser_key.equals("distance")){
    d.distance = (int)(value.toFloat()*100);
  }
  parser_key = "";
}

void Narodmon::endArray() {
}

void Narodmon::endObject() {
  if (values[values_cnt].type >= 0 && values[values_cnt].value != VALUE_NONE){
    DEBUG("%u------------", values_cnt);
    if (values_cnt < SENSOR_COUNT_LIMIT){
      values_cnt++;
      startObject();
    }
  }
}

int compare_by_value(const void* v1, const void* v2){
  const sensor_value *a = (sensor_value*)v1, *b = (sensor_value*)v2;
  return a->value - b->value;
}

int compare_by_distance(const void* v1, const void* v2){
  const sensor_value *a = (sensor_value*)v1, *b = (sensor_value*)v2;
  return a->distance - b->distance;
}

int16_t resolve_value(sensor_value* values, int values_cnt, int type, RESOLVE_MODE resolve_mode){
  sensor_value sensors[SENSOR_COUNT_LIMIT];
  int cnt = 0;
  for (int i=0; i<values_cnt; i++){
    if (values[i].type == type){
      sensors[cnt++] = values[i];
    }
  }
  
  if (cnt > 0){
    switch (resolve_mode){
      case MIN:
        qsort(sensors, cnt, sizeof(sensor_value), compare_by_value);
        return sensors[cnt > 1 ? 1 : 0].value;  //по возможности убираем крайние значения выборки
      case MAX:
        qsort(sensors, cnt, sizeof(sensor_value), compare_by_value);
        return sensors[cnt > 1 ? cnt-2 : cnt-2].value;  //по возможности убираем крайние значения выборки
      case AVG:{
        qsort(sensors, cnt, sizeof(sensor_value), compare_by_value);
        int b_index = 0, e_index = cnt;
        if (cnt > 2){  //по возможности убираем крайние значения выборки
          b_index = 1;
          e_index = cnt-1;
        }
        int32_t sum = 0;
        for (; b_index < e_index; b_index++){
          sum += sensors[b_index].value;
        }
        return sum / (e_index - b_index);
      }
      case CLOSEST:
        qsort(sensors, cnt, sizeof(sensor_value), compare_by_distance);
        return sensors[0].value;
    }
  }

  return VALUE_NONE;
}

void Narodmon::endDocument() {
  DEBUG("End document");
  int16_t value = resolve_value(values, values_cnt, NARODMON_TYPE_TEMPERATURE, MIN);
  if (value != VALUE_NONE){
    t = value;
    t_time = millis();

    DEBUG("T=%f", getT());
  }

  value = resolve_value(values, values_cnt, NARODMON_TYPE_HUMIDITY, CLOSEST);
  if (value != VALUE_NONE){
    h = value;
    h_time = millis();

    DEBUG("H=%f", getH());
  }

  value = resolve_value(values, values_cnt, NARODMON_TYPE_PRESSURE, CLOSEST);
  if (value != VALUE_NONE){
    p = value;
    p_time = millis();

    DEBUG("P=%f", getP());
  }

  AsyncClient * client = aClient;
  if (client){
    client->close();
  }
}

void Narodmon::startArray() {
}

void Narodmon::startObject() {
  values[values_cnt].type = -1;
  values[values_cnt].value = VALUE_NONE;
  values[values_cnt].distance = d.distance;
}
