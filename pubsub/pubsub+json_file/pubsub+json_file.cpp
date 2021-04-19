/* 
 *  PUBSUB - AWS
 *  
 *  Funcionamiento: Tarejeta ESP32 se conecta a AWS IoT core por medio de MQTT 
 *                  para publicar mensajes por un tópico y recibir 
 *                  mensajes por otro tópico
 *  
 *  Basado en el ejemplo de Anthony Elder
 *  https://github.com/HarringayMakerSpace/awsiot/blob/master/Esp32AWSIoTExample/Esp32AWSIoTExample.ino
 *  
 * Autor: Renzo Varela 
 * Fecha: 18-04-2021
 * License: --
 */


#include <WiFiClientSecure.h>
#include <PubSubClient.h> // install with Library Manager, I used v2.6.0
#include <ArduinoJson.h>
#include "certificados.h"
#include <EEPROM.h>

// CREDENCIALES WIFI Y AWS

WiFiClientSecure wiFiClient;
void msgReceived(char* topic, byte* payload, unsigned int len);
PubSubClient pubSubClient(awsEndpoint, 8883, msgReceived, wiFiClient); 

DynamicJsonBuffer jsonPubBuff;  // Buffer para los JSON enviados
DynamicJsonBuffer jsonSubBuff;  // Buffer para JSON recibido


//NOMBRE DE LOS TOPICOS
#define IN_TOPIC "LED"
#define OUT_TOPIC_1 "SERVO"


void setup() {
  
  Serial.begin(115200); delay(50); Serial.println();
  Serial.println("Basic pub-sub firmware v2.2");
  Serial.print("IN TOPIC: ");
  Serial.println(IN_TOPIC);
  //Serial.print("OUT TOPIC: ");
  //Serial.println(OUT_TOPIC);
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  WiFi.waitForConnectResult();
  Serial.print(", WiFi connected, IP address: "); Serial.println(WiFi.localIP());

  wiFiClient.setCACert(rootCA);
  wiFiClient.setCertificate(certificate_pem_crt);
  wiFiClient.setPrivateKey(private_pem_key);

  
}

unsigned long lastPublish;
long msgCount;

// Definicion del payload (JSON) que se va a enviar en cada topico
JsonObject& payload1 = jsonPubBuff.createObject();

void loop() {

  //CHECKEA CONEXION MQTT CON AWS
  pubSubCheckConnect();
  
  if (millis() - lastPublish > 10000) {
    msgCount++;
    //CONSTRUYE NUEVO JSON
    payload1["estado"]= 0;
    payload1["PWM"]= 511;
    payload1["number"]=msgCount;
    
    // CAMBIA EL FORMATO DEL PAYLOAD
    String sPayload;
    char* cPayload;    
    sPayload = "";
    payload1.printTo(sPayload);
    cPayload = &sPayload[0u]; 
  
    //PUBLISH TOPIC        
    //String msg = String("{\nHello from ESP8266: \n}\n"); //+ ++msgCount;
    boolean rc = pubSubClient.publish(OUT_TOPIC_1, cPayload);
    Serial.print("Published, rc="); Serial.print( (rc ? "OK: " : "FAILED: ") );
    Serial.println(cPayload);
    lastPublish = millis();
    }

  
}


void msgReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on "); Serial.print(topic); Serial.print(": ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

//CONECION CON EL SERVIDOR (AWS)
void pubSubCheckConnect() {
  if ( ! pubSubClient.connected()) {
    Serial.print("PubSubClient connecting to: "); Serial.print(awsEndpoint);
    while ( ! pubSubClient.connected()) {
      Serial.print(".");
      pubSubClient.connect(DEVICENAME);
      delay(1000);
    }
    Serial.println(" connected");
    pubSubClient.subscribe(IN_TOPIC);
  }
  pubSubClient.loop();
}
