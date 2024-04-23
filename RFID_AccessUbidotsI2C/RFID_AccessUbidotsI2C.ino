#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Ubidots.h"
#include "WiFi.h"
#include <WiFiMulti.h>
#include <WebSocketsClient.h>

#define SS_PIN 5  
#define RST_PIN 4 
#define lock 1 //tx
#define G 2   
#define R 0 
#define bl 15 

// Replace with your network credentials
const char* ssid = "";
const char* password = "";

// Cambiar para configurar la camara
IPAddress local_IP(192, 168, 1, 101);
IPAddress gateway(192, 168, 1, 254);

// No cambiar
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);

const char* UBIDOTS_TOKEN = "BBUS-JxClPItFeEBnVVfn6maudfEIQJAa0t";  // Ubidots Token
const char* WIFI_SSID = "";      // SSID de tu WiFi
const char* WIFI_PASS = "";      // Clave de tu WiFi
const char* DEVICE_LABEL = "rfid";      // Nombre del dispositivo

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
Ubidots ubidots(UBIDOTS_TOKEN, UBI_UDP);

// Setear el Tipo de LCD, 16x2 en este caso
LiquidCrystal_I2C lcd(0x27, 16, 2);

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Crear instancia para sensor RFID MFRC522

const int cantidad = 4; // Elegir la cantidad de llaves con Acceso.

/////////Tarjetas con Acceso/////////////

String pases[cantidad] = {"F3 05 15 FB","03 F5 50 DD", "39 3B 55 B8", "47 4E 87 62"}; //Codigos de tus tarjetas RFID que tendran acceso a entrar.
String nombres[cantidad] = {"  RAYMUNDO","     ANGEL", "     BRYAN", "    CARIDAD"}; //Modificar los nombres a tu gusto.

/////////////////////////////////////////

int acceso = 0;
int out = 0;
String llave;
int x= -1 ;


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

	switch(type) {
		case WStype_DISCONNECTED:
			Serial.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED:
			Serial.printf("[WSc] Connected to url: %s\n", payload);

			// send message to server when Connected
			webSocket.sendTXT("Connected");
			break;
		case WStype_TEXT:
			Serial.printf("[WSc] get text: %s\n", payload);

			// send message to server
			// webSocket.sendTXT("message here");
			break;
		case WStype_BIN:
			Serial.printf("[WSc] get binary length: %u\n", length);

			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
	}

}

void entra() {
  Serial.println("Aceptado");
  lcd.clear();
  digitalWrite(G, LOW);
  digitalWrite(lock, LOW);
  digitalWrite(bl, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("  BIENVENIDO:");
  lcd.setCursor(0, 1);
  lcd.print(nombres[x]);
  delay(2000); 
  webSocket.sendTXT("Foto");
  delay(2000);
  digitalWrite(G, HIGH);
  digitalWrite(lock, HIGH);
  digitalWrite(bl, LOW);
  lcd.clear();
}

void noentra() {
  Serial.println("No va a entrar");
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("  DESCONOCIDO");
  digitalWrite(bl, HIGH);
  digitalWrite(R, LOW);
  delay(2000);
  digitalWrite(R, HIGH);
  digitalWrite(bl, LOW);
  lcd.clear();

}

void verifico() {
  for (int y = 0; y <cantidad; y++) {
    //Serial.println(y);
    if (llave == pases[y]) {
      Serial.println(llave);
      //Serial.println(pases[y]);
      acceso = 1;
      x = y;
    }
  }
}

void registro() {
  
  Serial.println("Registrando");
  
  char str[11];
  
  nombres[x].toCharArray(str, sizeof(nombres[x]));

  
  ubidots.addContext("Entró", str);

  char* context = (char*)malloc(sizeof(char) * 60);

  /* Estructura el contexto para ser enviado */
  ubidots.getContext(context);

  /* Envia la variable con su contexto */
  ubidots.add("entradas", 1, context);  // Cambiar con el nombre de tu variable

  bool bufferSent = false;
  bufferSent = ubidots.send(DEVICE_LABEL);  // Enviará el dato al DEVICE_LABEL declarado.
  free(context);
}

/****************************************
   Funciones principales
 ****************************************/

void setup()
{
   // Serial port for debugging purposes
  Serial.begin(115200);

  WiFi.config(local_IP,gateway,subnet,dns);
  // Connect to Wi-Fi
  WiFiMulti.addAP(ssid, password);
  while(WiFiMulti.run() != WL_CONNECTED) {
		delay(100);
	}

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());  
  
  // server address, port and URL -------- Configurar la ip para conectar a la camara para tomar foto, no cambiar el puerto ni la url
	webSocket.begin("192.168.1.100", 80, "/ws");
	// event handler
	webSocket.onEvent(webSocketEvent);
  // try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);

  ubidots.wifiConnect(WIFI_SSID, WIFI_PASS);
  Wire.begin(21,22); // Iniciar I2C
  SPI.begin();      // Iniciar SPI
  mfrc522.PCD_Init();   // Iniciar MFRC522
   Serial.print("Iniciado");
  pinMode(R, OUTPUT);
  digitalWrite(R, HIGH);
  pinMode(G, OUTPUT);
  digitalWrite(G, HIGH);
  pinMode(lock, OUTPUT);
  digitalWrite(lock, HIGH);
  pinMode(bl, OUTPUT);
  lcd.begin();
  lcd.setCursor(0, 0);
  lcd.print(" SERVIDOR UNAM 1");
  
}
void loop()
{ 
  webSocket.loop();
  lcd.setCursor(0, 0);
  lcd.print(" SERVIDOR UNAM 1");
  lcd.setCursor(0, 1);
  lcd.print(" MUESTRE LLAVE");
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Show UID on serial monitor
  //Serial.println();
  //Serial.print(" UID tag :");
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    //Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  //Serial.println();

  llave = content.substring(1);
  
  verifico();

  if (acceso==1) 
  {
    entra();
    registro();
    acceso=0;
  }

  else   {
    noentra();
    acceso=0;
  }
}

