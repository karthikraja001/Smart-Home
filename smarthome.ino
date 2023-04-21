#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT11
#include <FirebaseESP8266.h>

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#define D2 4
#define D1 5
#define D3 0
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "WIFINAME"
#define WIFI_PASSWORD "WIFIPASS"

// Insert Firebase project API Key
#define API_KEY "YOUR_FRIEBASE_API_KEY"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "YOUR_FIREBASE_RTDB_URL" 
//Define Firebase Data object
FirebaseData fbdo;
 
FirebaseAuth auth;
FirebaseConfig config;
 
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
bool isOn = false;

DHT dht(DHTPIN, DHTTYPE);


void setup() {
  Serial.begin(115200);
  pinMode(4, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D3, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");  
  Serial.println(WiFi.localIP());
  Serial.println();
 
  /* Assign the api key (required) */
  config.api_key = API_KEY;
 
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
 
  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", "config.signer");
  }
 
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  delay(2000);
}

int timeSinceLastRead = 0; 
void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    bool toReadTemp = Firebase.RTDB.getBool(&fbdo, "toReadTemp");
    if(fbdo.boolData() == true){
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      float f = dht.readTemperature(true);

      if (isnan(h) || isnan(t) || isnan(f)) {
        Serial.println("Unable to read Temperature");
        timeSinceLastRead = 0;
        return;
      }

      float hif = dht.computeHeatIndex(f, h);
      float hic = dht.computeHeatIndex(t, h, false);

      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.println(" *C ");

      timeSinceLastRead = 0;
      if(Firebase.RTDB.setFloat(&fbdo, "humidity", h)){
        Serial.println("Humidity Wrote in DB");
      }
      if(Firebase.RTDB.setFloat(&fbdo, "temperature", t)){
        Serial.println("Temperature Wrote in DB");
      }
      if(Firebase.RTDB.setFloat(&fbdo, "heatIndex", hic)){
        Serial.println("Heat Index Wrote in DB");
      }
      Serial.println("__________________________________________________");
    }else{}

    bool toReadFans = Firebase.RTDB.getBool(&fbdo, "fans");
    if(fbdo.boolData() == true){
      digitalWrite(D1, HIGH);
    }else{
      digitalWrite(D1, LOW);
    }

    bool toReadLight = Firebase.RTDB.getBool(&fbdo, "lights");
    if(fbdo.boolData() == true){
      digitalWrite(D3, HIGH);
    }else{
      digitalWrite(D3, LOW);
    }
  }
}
