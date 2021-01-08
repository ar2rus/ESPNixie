#include "SerialDebug.h"

#include "Arduino.h"

WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

void serailDebug_init(){
  server.begin();
  server.setNoDelay(true);

}

void serailDebug_update(){
  
   uint8_t i;
  //check if there are any new clients
  if (server.hasClient()){
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()){
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  
}

void _print(String text){
   for(int i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        serverClients[i].print(text);
        delay(1);
      }
    }
}

void _println(String text){
   for(int i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        serverClients[i].println(text);
        delay(1);
      }
    }
}

void _print(int v){
   for(int i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        serverClients[i].print(v);
        delay(1);
      }
    }
}

void _println(int v){
   for(int i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        serverClients[i].println(v);
        delay(1);
      }
    }
}
