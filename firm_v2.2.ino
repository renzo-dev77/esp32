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
 * Fecha: 17-01-2021
 * License: --
 */


#include <WiFiClientSecure.h>
#include <PubSubClient.h> // 
#include <ArduinoJson.h> // version 5.x.x
#include "certificados.h"
#include <EEPROM.h>

// CREDENCIALES WIFI Y AWS
const char* DEVICENAME = "ESP32_001";
const char* ssid = "VTR-9523911";
const char* password = "xn2Zpdgphnjx";
const char* awsEndpoint = "a2i93w7eha7z2s-ats.iot.us-west-2.amazonaws.com";

WiFiClientSecure wiFiClient;
void msgReceived(char* topic, byte* payload, unsigned int len);
PubSubClient pubSubClient(awsEndpoint, 8883, msgReceived, wiFiClient); 

DynamicJsonBuffer jsonPubBuff;  // Buffer para los JSON enviados
DynamicJsonBuffer jsonSubBuff;  // Buffer para JSON recibido


//NOMBRE DE LOS TOPICOS
#define IN_TOPIC "deviceStatus/ESP32_Test2"
#define OUT_TOPIC_1 "hearbeat"
#define OUT_TOPIC_2 "chargeEvent"


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

  // CONSTRUYE UN NUEVO JSON     
  // payload1["status"]= "on";
  ++msgCount;
  payload1["idDevice"]= DEVICENAME;
  payload1["number"]=msgCount;
    
  // CAMBIA EL FORMATO DEL PAYLOAD
  String sPayload;
  char* cPayload;    
  sPayload = "";
  payload1.printTo(sPayload);
  cPayload = &sPayload[0u];
  Serial.println(cPayload);    

  //PUBLISH TOPIC
  pubSubClient.publish(OUT_TOPIC_1, cPayload);

  lastPublish = millis();
  
}

//FUNCION PARA ENCENDER CARGADOR
void onLight(){
  digitalWrite(23, HIGH); 
  Serial.println("ON LIGHT");
  // ACTUALIZA LA VARIABLE DE ESTADO DEL CARGADOR
  payload1["status"]= "on";
  }


//FUNCION PARA RECIBIR MENSAJES DESDE EL SERVIDOR (AWS)   
void msgReceived(char* topic, byte* payload, unsigned int length) {

  // DEFINICION DEL JSON QUE SE RECIBE
    JsonObject& jsonReceived = jsonSubBuff.parseObject(payload);

  // ASIGNA VARIABLES LOCALES A LOS CAMPOS DEL JSON RECIBIDO
  const char* idDevice = jsonReceived["idDevice"];
  const char* idUser = jsonReceived["idUser"];
  const char* eventChargeId = jsonReceived["eventChargeId"];
  String status_ = jsonReceived["status"];


  // CONSTRUYE EL NUEVO JSON PARA ENVIAR
  JsonObject& payload2 = jsonPubBuff.createObject();
  payload2["eventChargeId"]= eventChargeId;
  payload2["idDevice"]= idDevice;
  payload2["status"]= status_;

  // CAMBIA EL FORMATO DEL PAYLOAD
  String sPayload;
  char* cPayload;    
  sPayload = "";
  payload2.printTo(sPayload);
  cPayload = &sPayload[0u];
  Serial.println(cPayload);    

  //PUBLISH TOPIC  
  pubSubClient.publish(OUT_TOPIC_2, cPayload);

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
