#include "OneWire.h"
#include "DallasTemperature.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>

const int ledPin =19;
String ledState = "OFF";
float termoSensorValue;
int lightSensorValue = 0;

//remplacer par son adresse IP
String ipServeur ="192.168.43.66";


OneWire onWire(23); //lecture du capteur de temperateur sur le pin 23
DallasTemperature tempSensor(&onWire);

//Remplacer par son adresse IP
IPAddress server(192,168,168,66);

WiFiClient client;
WebSocketsClient webSocket;

void print_status(){
  Serial.print("WiFi connected \n");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print("\n");
  Serial.print("MAC address: ");
  Serial.print(WiFi.macAddress());
  Serial.print("\n"); 
}

float getTemperature()
{
  tempSensor.requestTemperaturesByIndex(0); // le capteur realise une acquisition
  return tempSensor.getTempCByIndex(0);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  char * c = new char[1];
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WS] Disconnected !\n");
      break;
    case WStype_CONNECTED:
      Serial.printf("[WS] Connected to url: %s\n", payload);
      // send message to server when Connected
      webSocket.sendTXT("Arduino");
      break;
    case WStype_TEXT:
      Serial.println("reçus case");
      Serial.printf("[WS] get text: %s\n", payload);
      c = (char *)payload;
      wsMessages(c[0]);
      break;

    case WStype_ERROR:      
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }

}
void wsMessages(char inChar) {
  Serial.print("Requete reçu : ");
  Serial.println(inChar);  
  String returnMessage;
  
  switch (inChar) {
    case '0' :
      returnMessage = "ip/"+WiFi.localIP().toString();      
      delay(1000);
      Serial.println("");
      Serial.print("Message envoyé : ");
      Serial.print(returnMessage);
      webSocket.sendTXT(returnMessage);
      delay(1000);
      break;
    case '1' :
      returnMessage = "mac/"+WiFi.macAddress();      
      delay(1000);
      Serial.println(""); 
      Serial.print("Message envoyé : ");
      Serial.print(returnMessage);
      webSocket.sendTXT(returnMessage);
      delay(1000);
      break;
    case '2' :
      if(ledState == "ON"){
        digitalWrite(ledPin, LOW);
        ledState = "OFF";
      }else if(ledState == "OFF"){
        digitalWrite(ledPin, HIGH);
        ledState = "ON";
      }
      delay(1000);
      Serial.println("");
      Serial.print(ledState);
      returnMessage = "led/"+ledState;
      Serial.print("Message envoyé : ");
      Serial.print(returnMessage);
      webSocket.sendTXT(returnMessage);
      delay(1000);
      break;
    case '3' :
      termoSensorValue = getTemperature();
      delay(1000);
      Serial.println(""); 
      Serial.println(termoSensorValue,DEC);
      webSocket.sendTXT("temp/"+String(termoSensorValue));
      Serial.print("Message envoyé : ");
      Serial.print(returnMessage);
      webSocket.sendTXT(returnMessage);
      delay(1000);
      break;
    case '4' :
      lightSensorValue = analogRead(A0);
      delay(1000);
      Serial.println(""); 
      Serial.println(lightSensorValue,DEC);
      returnMessage = "light/"+String(lightSensorValue);
      Serial.print("Message envoyé : ");
      Serial.print(returnMessage);
      webSocket.sendTXT(returnMessage);
      delay(1000);
      break;
    default:
      Serial.println(""); 
  }
}

void connect_wifi(){
 //const char* ssid = "HUAWEI-6EC2";
 //const char *password= "FGY9MLBL"; 
 const char* ssid = "Pixel_3822";
 const char* password= "LeBonMotDePasse"; 
 
 Serial.println("Connecting Wifi...");
 WiFi.begin(ssid, password);
 while(WiFi.status() != WL_CONNECTED){
   Serial.println("Attempting to connect ..");
   delay(1000);
 }
 
 Serial.print("Connected to local Wifi\n");  
 print_status();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); // Begin port to reach
  pinMode(ledPin,OUTPUT);
  tempSensor.begin();
  
  connect_wifi();
  print_status();

  webSocket.begin(ipServeur, 8080, "/");
  Serial.println("Arduino");
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
}
