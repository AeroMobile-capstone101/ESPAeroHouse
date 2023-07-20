#include <WiFi.h>
#include <ESPping.h>
#include <DHT.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

// pins
DHT in_dht(13, DHT11);
#define MIST_PIN 2
#define BLUE_PIN 21
#define RED_PIN 22
#define PH_PIN 32

#define API_KEY "AIzaSyAMmI6VKqUBpgd9fAGnJpbkCk33UuTznDo"
#define FIREBASE_PROJECT_ID "aeromobile-906d2"
#define USER_EMAIL "mikognrl@gmail.com"
#define USER_PASSWORD "123456"
#define FIREBASE_CLIENT_EMAIL "firebase-adminsdk-t3ngv@aeromobile-906d2.iam.gserviceaccount.com"

const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQDWdc02SFqEIlXC\n/BHSdsDrFvns8SaWkUNjRxSdRYB4qR/mUd0Hn52ZmutxhsnBF/0Xv9p4YhXngn7V\ntmrPnLuiNee3kYPfu0bgoO82bE/V3O6j31DoiMioFnrEx0g4HDnYGr0UhCYd3L48\nTZLzWv5vTiNmk4cXFRGdbUebk/a73+osUIOM+NASIHQ9KQDUAD/Frg5Uqcw/y2HF\nxUFhw1cv7xYtmjI9IIgNdlk1C7WFZjCvDO1D9oSdr+h+vELFU6rNXbWIzQUAnCpq\nT3kILtT82YDKTJ3UxDognTKaJJluvJDO8RcJ4gcNUXoqB5S9yIC5L5PVHgWXuoA9\nvT/EBV4pAgMBAAECggEAAUFEN6aZvP80cEUoV4pxW47RXJUWrDfiZJDUao0vEMWw\nlZjyCarh8bWp3gqzYIli5kItpZmxVh753gdIvblFkc/t8LGNy3/djM/0RmXA29gE\nn3ndj9bJEc673i3qWsWzgvO5Gochb/hMNq8fhoQITB9xbpa7aiOp7Dlo+1yUy4mj\npVjesmUB52fsMrA07czbZy2i0wKcHiMIzQV0JWYN2gcHW3KUwGXdl37oOXKM0qsV\nFHFWoI1K+0nE/y3OZp1S78eeZPHy2CZazBDRtAo5xNKTj8XE7uid8ksy8jD8ThRv\nKF6lIdxs2rx7w3J4AH32JY9Eq6t4poOTeXBUjesMQQKBgQD+DIGTw4ZFvwvokfYG\npVNEzkA8KUZATC71tM0x2+/VbsWTB+8d320QRNwNtnfHGvE4WMVoqIpcgSqfIc+N\nTl3st1Uv0HFwZz8mJpJVDMmTbXQAdZ5C7du0PjvIb5o8i9pOYJMTnKVX+8Lz/GtU\nL986XuhgnQW0iv3AO0UWFxLV0QKBgQDYG3V1HhaODcW0+130gOztl5yL+wlYDYsb\nl066grBLS8F0JLVLHjT6C7DYG5lsxKMyusxPnHonDlWziR0NTGvcuTV2/eLlo5PK\nPfy17sKBRxJ4a9LupHR37PbIXKa8TNzCupkWx/vJ/vlUfjoojJm+1CkB3OGy2oVa\nM1R0RhQg2QKBgFaPeE8andOB5tLVeNiG2DpndCeLgK5DxvRdKFVGtQ8p8RPJ7kVl\nbUnPzJK3i1RB79LdHFYupwYh1J3n7UoeW45eYR4rjZtY8oFTPyTPeJdNAetuKABx\n/xwI4GHI9OqQVE/ERBi4lPXHiIONjBxhO1QuJktU9S6w9QjcqQt/zSEhAoGAEXku\n5xhWtIeyi66jayidf5x5oOvcVo1JySJT0ErO3Fe0vaJWMkuv9uPJGcApiJzhoTFv\nYaSCT/Qdw9V7Tt71/bPP+d4PYYf3ZCvc6gzzFyba/HejZ35OmH/FeG/dX25Lj0ct\nJ6Vl23BNUqKeKzcrGz3mTutAKIj4rVFnK7ua1wkCgYARleZsHG407uykN4eDa+qZ\nB4h9XCK5PWWWsnhHP6hLUHB9cEhm3YzMi9fQaWD+JYyOfT2Vizl1hdd1bK1VEv1m\n0M74xyFiFIJJsuUQywW/2eRQ0/aYJCKh+XeRZGwsn5tgpSennTvhs1NtsYG/bYe+\nFP4FuqXNRFvFgfoOPIgY1Q==\n-----END PRIVATE KEY-----\n";
const char* ntpServer = "pool.ntp.org";
const int gmtOffset_sec = 28800;
const int daylightOffset_sec = 3600;
bool redLedState = true;
bool blueLedState = true;
bool mistMakerState = true;
bool isWebServerRunning = false;

unsigned long mistMakerToggleMillis = 0;
unsigned long misterOnTime = 300000;
unsigned long misterOffTime = 300000;
unsigned long paramReadingInterval = 10000;
unsigned long paramReadingMillis = 0;
unsigned long phReadingMillis = 0;
unsigned long phReadingInterval = 10000;
unsigned long dbUpdateMillis = 0;
unsigned long wifiCheckMillis = 0;


float temperatureValue = 0;
float humidityValue = 0;
float pHValue = 0;

float averageTemp = 0;
float averageHumid = 0;
float averagePH = 0;
String wifi_ssid = "";
String wifi_password = "";

float phReadingsCount = 0;
float readingsCount = 0;
float readingsSumHumidity = 0;
float readingsSumInTemperature = 0;
float readingsSumPH = 0;
unsigned long averageReadingInterval = 1800000;
unsigned long averageReadingMillis = 0;

struct tm timeinfo;
int currentHour = 0;

StaticJsonDocument<256> doc;
String config_filename = "/config.json";

// Firebase Object Instantation
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String houseName = "AeroHouse_4.0";
String documentPath = "system_collection/" + houseName;

WebServer server(80);

// function prototype
void connectWiFi();
void switchToAPMode();
void mistMakerControl();
void LedControl();
void paramReading();
void handleRoot();
void handleSaveWiFiCredentials();
void handlesaveMisterConfig();
void handleWiFiConnect();
void handleMisterConfig();
void toggleMistMakers(bool state);
void paramReading(unsigned long currentMillis);
void dbGetSendFirestore(unsigned long currentMillis);
void updateSummaryArray(String timeOfTheDay);
void updateParameterFields();
void checkUploadSummary();
void sendNotification();
bool readConfigSPIFFS();
bool saveConfigSPIFFS();
bool saveWiFiCredentialsSPIFFS();
bool saveAverageParamsSPIFFS();
bool saveMisterConfigSPIFFS();
bool writeFile(fs::FS& fs, String filename, String message);
float phRead();
float getMistOff();
float getMistOn();
String getDeviceIDToken();
String readFile(fs::FS& fs, String filename);
String SendHTML();
String refreshPage(int time);
String getLightColor();

// ------------------ SETUP and LOOP ----------------------

void setup() {
  Serial.begin(115200);
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(MIST_PIN, OUTPUT);
  pinMode(PH_PIN, INPUT);

  in_dht.begin();  // setup dht
  delay(1000);


  // if (!SPIFFS.begin(true)) {
  //   Serial.println("setup -> SPIFFS Mount Failed");
  // } else {
  //   Serial.println("setup -> SPIFFS Mounted Successfully");

  //   if (!readConfigSPIFFS()) {
  //     Serial.println("setup -> Could not read Config file -> initializing new file");
  //     if (saveConfigSPIFFS()) {
  //       Serial.println("setup -> Config file saved");
  //     }
  //   }
  //   Serial.println("setup -> readFile Successful");
  //   connectWiFi();
  // }
}

void loop() {
  unsigned long currentMillis = millis();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi Connected...");

    if (!Ping.ping("www.google.com")) {
      switchToAPMode();
      Serial.println("AP Mode is switch due to wifi has no internet...");
      return;
    }

    if (isWebServerRunning) {
      server.stop();                // stop web server
      WiFi.softAPdisconnect(true);  // disable access point mode
      connectWiFi();                // connect the wifi back
      isWebServerRunning = false;
      Serial.println("Web server and Wifi AP is stop...");
    }

    if (Firebase.ready()) {
      mistMakerControl(currentMillis);
      paramReading(currentMillis);
      dbGetSendFirestore(currentMillis);
      LedControl();
    }
    //  mistMakerControl(currentMillis);
    delay(500);
    return;
  }

  // only runs if WiFi status is not connected and has no internet
  server.handleClient();
  mistMakerControl(currentMillis);
  paramReading(currentMillis);
  delay(500);
}



// ------------------------ WIFI Section --------------------------


void connectWiFi() {
  if (wifi_ssid.length() <= 0 && wifi_password.length() <= 0) {
    Serial.print("No Wifi Credentials Found in SPIFFS!...");
    return;
  }

  WiFi.mode(WIFI_STA);  // set the esp wifi to station mode

  if (wifi_ssid.length() > 0 && wifi_password == "") {
    WiFi.begin(wifi_ssid.c_str());
  } else {
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  }

  Serial.print("Connecting to Wi-Fi..");

  byte attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    Serial.print(".");
    delay(1000);
    attempts++;
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect wifi...");
    switchToAPMode();
    return;
  }

  configFirebase();  // initialize Firebase connections
}

void switchToAPMode() {
  Serial.println("ESP switch to AP_MODE");
  IPAddress local_ip(192, 168, 1, 1);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress mask(255, 255, 255, 0);

  const char* ssid = "AeroHouse_AP";

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid);
  WiFi.softAPConfig(local_ip, gateway, mask);
  delay(200);

  server.begin();
  server.on("/", handleRoot);
  server.on("/saveWiFiCredentials", handleSaveWiFiCredentials);
  server.on("/connectWiFi", handleWiFiConnect);
  server.on("/configureMister", handleMisterConfig);
  server.on("/saveMisterConfg", handlesaveMisterConfig);
  digitalWrite(RED_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  delay(500);

  isWebServerRunning = true;
}

// ------------------------ Firebase Section -------------------------

void configFirebase() {
  Serial.println("Firebase Configuration.....");
  // Assign the api key (required)
  config.api_key = API_KEY;

  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  //Assign the sevice account credentials and private key (required)
  config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
  config.service_account.data.project_id = FIREBASE_PROJECT_ID;
  config.service_account.data.private_key = PRIVATE_KEY;

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h
  fbdo.setResponseSize(10000);
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  // include config time of ntp server
}

void dbGetSendFirestore(unsigned long currentMillis) {
  if (currentMillis - dbUpdateMillis >= 10000) {
    if (getLightColor() == "red") {
      redLedState = true;
      blueLedState = false;
    } else if (getLightColor() == "blue") {
      redLedState = false;
      blueLedState = true;
    } else if (getLightColor() == "purple") {
      blueLedState = true;
      redLedState = true;
    } else {
      blueLedState = false;
      redLedState = false;
    }

    unsigned long mistOn = getMistOn();
    unsigned long mistOff = getMistOff();

    if (mistOn != misterOnTime || mistOff != misterOnTime) {
      misterOnTime = mistOn;
      misterOffTime = mistOff;
      Serial.println("dbGetSendFirestore() function");
      Serial.printf("-------------------------- GET MISTER ON = %d------------------------------/n,", misterOnTime);
      Serial.printf("-------------------------- GET MISTER OFF = %d------------------------------/n", misterOffTime);
      saveMisterConfigSPIFFS();
    }

    updateParameterFields();
  }

  checkUploadSummary();
}

void checkUploadSummary() {
  int uploadTimes[] = { 7, 19 };  //advance in one hour to fit in Philippine Standard Time 6am and 6pm during upload
  bool uploadScheduled = false;

  String timeOfTheDay = "";

  for (int i = 0; i < sizeof(uploadTimes) / sizeof(uploadTimes[i]); i++) {
    if (currentHour == uploadTimes[i]) {
      uploadScheduled = true;
      break;
    }
    if (uploadTimes[i] == 7) {
      timeOfTheDay = "AM";
    } else {
      timeOfTheDay = "PM";
    }
  }


  if (uploadScheduled) {
    getAverage();
    updateSummaryArray(timeOfTheDay);
    uploadScheduled = false;
  } else {
    Serial.println("Array upload scheduled for 6 AM and 6 PM");
  }
}

//---------------------------- SEND DATA TO FIREBASE -----------------------------

void updateSummaryArray(String timeOfTheDay) {
  Serial.print("Commit a document (append array)... ");

  // The dyamic array of write object fb_esp_firestore_document_write_t.
  std::vector<struct fb_esp_firestore_document_write_t> writes;

  // A write object that will be written to the document.
  struct fb_esp_firestore_document_write_t transform_write;

  transform_write.type = fb_esp_firestore_document_write_type_transform;

  // Set the document path of document to write (transform)
  transform_write.document_transform.transform_document_path = documentPath;

  // Set a transformation of a field of the document.
  struct fb_esp_firestore_document_write_field_transforms_t field_transforms;

  // Set field path to write.
  field_transforms.fieldPath = "summary";

  field_transforms.transform_type = fb_esp_firestore_transform_type_append_missing_elements;

  FirebaseJson content;


  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }

  char date[11];
  strftime(date, sizeof(date), "%Y-%m-%d", &timeinfo);

      Serial.println("updateSummaryArray() function");
      Serial.printf("-------------------------- MISTER ON = %d------------------------------, misterOnTime");
      Serial.printf("-------------------------- MISTER OFF = %d------------------------------, misterOffTime");

  content.set("values/[0]/mapValue/fields/created_at/stringValue", date);
  content.set("values/[0]/mapValue/fields/time_of_the_day/stringValue", timeOfTheDay);
  content.set("values/[0]/mapValue/fields/temperature/doubleValue", averageTemp);
  content.set("values/[0]/mapValue/fields/humidity/doubleValue", averageHumid);
  content.set("values/[0]/mapValue/fields/acidity/doubleValue", averagePH);
  content.set("values/[0]/mapValue/fields/mistingOn/doubleValue", misterOnTime);
  content.set("values/[0]/mapValue/fields/mistingOff/doubleValue", misterOffTime);
  content.set("values/[0]/mapValue/fields/lightColor/stringValue", getLightColor());


  // Set the transformation content.
  field_transforms.transform_content = content.raw();

  // Add a field transformation object to a write object.
  transform_write.document_transform.field_transforms.push_back(field_transforms);

  // Add a write object to a write array.
  writes.push_back(transform_write);

  if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
  else
    Serial.println(fbdo.errorReason());
}

void updateParameterFields() {

  // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
  FirebaseJson content1;

  // misterOnTime = getMistOn();
  // misterOffTime = getMistOff();
  Serial.println("updateParameterFields() function");
      Serial.printf("-------------------------- MISTER ON = %d------------------------------", misterOnTime);
      Serial.printf("-------------------------- MISTER OFF = %d------------------------------", misterOffTime);

  content1.set("fields/_temperature/doubleValue", temperatureValue);
  content1.set("fields/_humidity/doubleValue", humidityValue);
  content1.set("fields/_acidity/doubleValue", pHValue);
  content1.set("fields/_mistOffTime/doubleValue", misterOffTime);
  content1.set("fields/_mistOnTime/doubleValue", misterOnTime);
  content1.set("fields/_deviceToken/stringValue", getDeviceIDToken());
  content1.set("fields/_lightColor/stringValue", (getLightColor() == "") ? "off" : getLightColor());



  //Serial.print("Create document... ");

  if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath, content1.raw(), "_temperature,_humidity, _acidity, _mistOffTime, _mistOnTime, _deviceToken, _lightColor")) {
    Serial.println("");
    //Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
  } else {
    Serial.println(fbdo.errorReason());
  }

  //}
}


//------------------------------GET DATA FROM FIREBASE----------------------------

String getLightColor() {
  //getDocument lightColor
  String lightC = "";
  String fieldPath = "_lightColor";

  Serial.print("Get the lightColor... ");

  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), fieldPath.c_str())) {
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    // Create a FirebaseJson object and set content with received payload

    FirebaseJson payload;
    payload.setJsonData(fbdo.payload().c_str());
    FirebaseJsonData jsonData;
    payload.get(jsonData, "fields/_lightColor/stringValue", true);
    lightC = jsonData.stringValue;

  } else {
    Serial.println(fbdo.errorReason());
  }

  return lightC;
}

float getMistOff() {
  unsigned long mistOff=0;
  String fieldPath = "_mistOffTime";

  //Serial.print("Get the OFF mistingFrequency... ");

  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), fieldPath.c_str())) {
    Serial.println("");
    //Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    // Create a FirebaseJson object and set content with received payload
    FirebaseJson payload;

    payload.setJsonData(fbdo.payload().c_str());

    // Get the data from FirebaseJson object
    FirebaseJsonData jsonData;
    payload.get(jsonData, "fields/_mistOffTime/doubleValue", true);
    mistOff = jsonData.doubleValue;
    //Serial.print("Misting Frequency value: ");
    //Serial.println(mistOff);

  } else {
    Serial.println(fbdo.errorReason());
    mistOff=misterOffTime;
  }

  return mistOff;
}

float getMistOn() {
  unsigned long mistOn=0;
  String fieldPath = "_mistOnTime";

  Serial.print("Get the ON mistingFrequency... ");

  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), fieldPath.c_str())) {
    Serial.println("");
    //Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    // Create a FirebaseJson object and set content with received payload
    FirebaseJson payload;

    payload.setJsonData(fbdo.payload().c_str());

    // Get the data from FirebaseJson object
    FirebaseJsonData jsonData;
    payload.get(jsonData, "fields/_mistOnTime/doubleValue", true);
    mistOn = jsonData.doubleValue;
    //Serial.print("Misting Frequency value: ");
    //Serial.println(mistOn);

  } else {
    Serial.println(fbdo.errorReason());
    mistOn=misterOnTime;
  }

  return mistOn;
}

String getDeviceIDToken() {
  //getDocument Device ID token
  String ID = "";
  String fieldPath = "_deviceToken";

  Serial.print("Get the device ID Token... ");

  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), fieldPath.c_str())) {
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());

    FirebaseJson payload;
    payload.setJsonData(fbdo.payload().c_str());

    FirebaseJsonData jsonData;
    payload.get(jsonData, "fields/_deviceToken/stringValue", true);
    ID = jsonData.stringValue;
    Serial.println(ID);

  } else {
    Serial.println(fbdo.errorReason());
  }

  return ID;
}

void sendNotification() {

  Serial.print("Send Firebase Cloud Messaging... ");
  FCM_HTTPv1_JSON_Message msg;

  msg.token = getDeviceIDToken();


  if (temperatureValue < 17 || temperatureValue > 34) {

    if (temperatureValue < 17) {
      msg.notification.title = "Low Temperature Alert!" + houseName;
      msg.notification.body = "Temperature is below normal levels. Current temperature is " + String(temperatureValue);
    }

    else if (temperatureValue > 34) {
      msg.notification.title = "High Temperature Alert!" + houseName;
      msg.notification.body = "Temperature is above normal levels. Current temperature is " + String(temperatureValue);
    }


    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
    FirebaseJson payload;

    msg.data = payload.raw();

    if (msg.token == "") {
      return;
    }

    if (Firebase.FCM.send(&fbdo, &msg)) {
      Serial.printf("ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
    } else {
      Serial.println(fbdo.errorReason());
    }
  }

  if (humidityValue < 80) {

    msg.notification.title = "Low Humidity Alert!" + houseName;
    msg.notification.body = "Humidity is below normal levels. Current humidity: " + String(humidityValue);

    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
    FirebaseJson payload;

    msg.data = payload.raw();


    if (Firebase.FCM.send(&fbdo, &msg)) {
      Serial.printf("ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
    }

    else {
      Serial.println(fbdo.errorReason());
    }
  }

  if (pHValue < 5.5 || pHValue > 6.5) {

    if (pHValue < 5.5) {
      msg.notification.title = "Low Acidity Alert!" + houseName;
      msg.notification.body = "Acidity is below normal levels. Current acidity is " + String(pHValue);
    }

    else if (pHValue > 6.5) {
      msg.notification.title = "High Acidity Alert!" + houseName;
      msg.notification.body = "Acidity is above normal levels. Current acidity is " + String(pHValue);
    }

    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
    FirebaseJson payload;

    msg.data = payload.raw();


    if (Firebase.FCM.send(&fbdo, &msg)) {
      Serial.printf("ok\n%s\n\n", Firebase.FCM.payload(&fbdo).c_str());
    }

    else {
      Serial.println(fbdo.errorReason());
    }
  }
}

// -------------------------- Sensor Readings -----------------------

void paramReading(unsigned long currentMillis) {
  Serial.println("Param Reading Function");
  // if (currentMillis - phReadingMillis >= phReadingInterval){
  //   phReadingMillis = currentMillis;

  //   pHValue = (int)(phRead()*10) / 10.0;
  //   phReadingsCount++;
  //   readingsSumPH += pHValue;
    
  //   Serial.println("pH Level: " + String(pHValue) + "pH");
  // }
  if (currentMillis - paramReadingMillis >= paramReadingInterval) {
    temperatureValue = isnan(in_dht.readTemperature()) ? 0 : in_dht.readTemperature();
    humidityValue = isnan(in_dht.readHumidity()) ? 0 : in_dht.readHumidity();
    pHValue = (int)(phRead()*10) / 10.0;


    // upload parameters to firestore
    readingsCount++;

    readingsSumHumidity += humidityValue;
    readingsSumInTemperature += temperatureValue;

    Serial.print("Humidity: " + String(humidityValue) + "%");
    Serial.print("\tTemp In: " + String(temperatureValue) + "°C");

    paramReadingMillis = currentMillis;
  }
}

void getAverage(){
  averageHumid = (int)((readingsSumHumidity / readingsCount) * 100) / 100.0;
  averageTemp = (int)((readingsSumInTemperature / readingsCount) * 100) / 100.0;
  averagePH = (int)((readingsSumPH / phReadingsCount) * 10) / 10.0;

  readingsCount = 0;
  readingsSumHumidity = 0;
  readingsSumInTemperature = 0;
  readingsSumPH = 0;
  phReadingsCount = 0;

  Serial.println("---------------- AVERAGE ----------------");
  Serial.print(String(averageHumid) + "\t" + String(averageTemp) + "\t" + String(averagePH));
  Serial.println();

  saveAverageParamsSPIFFS();
}

float phRead() {

  // digitalWrite(RED_PIN, LOW);
  // digitalWrite(BLUE_PIN, LOW);
  // digitalWrite(MIST_PIN, LOW);

  // delay(3000);

  float adc_resolution = 4095.0;  //ESP 32 ADC Resolution
  int measurings = 0;
  for (int i = 0; i < 10; i++) {
    measurings += analogRead(PH_PIN);
    delay(10);
  }
  float voltage = 3.3 / adc_resolution * measurings / 10;
  return 7 + ((2.50 - voltage) / 0.18) + 1.3;
}

// -------------------- module control functions -----------------------------

void mistMakerControl(unsigned long currentMillis) {

  if (mistMakerState) {
    digitalWrite(MIST_PIN, mistMakerState);
    if ((currentMillis - mistMakerToggleMillis) >= misterOnTime) {
      mistMakerState = false;
      Serial.println("Mist Off");
      mistMakerToggleMillis = currentMillis;
    }
  } else {
    digitalWrite(MIST_PIN, mistMakerState);
    if ((currentMillis - mistMakerToggleMillis) >= misterOffTime) {
      mistMakerState = true;
      Serial.println("Mist On");
      mistMakerToggleMillis = currentMillis;
    }
  }
}

void LedControl() {
  if (redLedState) {
    digitalWrite(RED_PIN, HIGH);
  } else {
    digitalWrite(RED_PIN, LOW);
  }
  if (blueLedState) {
    digitalWrite(BLUE_PIN, HIGH);
  } else {
    digitalWrite(BLUE_PIN, LOW);
  }
}

// -------------------------- SPIFFS ----------------------------
bool saveConfigSPIFFS() {
  bool things = true;
  if (!saveWiFiCredentialsSPIFFS()) things = false;
  if (!saveAverageParamsSPIFFS()) things = false;
  if (!saveMisterConfigSPIFFS()) things = false;
  return things;
}

bool readConfigSPIFFS() {
  String file_content = readFile(SPIFFS, config_filename);

  int config_file_size = file_content.length();
  Serial.println("Config file size: " + String(config_file_size));
  Serial.println();

  if (config_file_size > 256) {
    Serial.println("Config file too large");
    return false;
  }

  auto error = deserializeJson(doc, file_content);
  if (error) {
    Serial.println("Error interpreting config file");
    return false;
  }

  Serial.println("readConfigSPIFFS() function");
      Serial.printf("-------------------------- MISTER ON = %d------------------------------\n, misterOnTime");
      Serial.printf("-------------------------- MISTER OFF = %d------------------------------\n, misterOffTime");

  String _wifi_ssid = doc["wifi_ssid"];
  String _wifi_password = doc["wifi_pass"];
  const long _misterOnTime = doc["mister_on_time"];
  const long _misterOffTime = doc["mister_off_time"];
  const float _aHumidity = doc["humidity"];
  const float _aTemperature = doc["temperature"];
  const float _aPH = doc["pH"];

  Serial.println("\nWi-Fi SSID from SPIFFS: " + _wifi_ssid);
  Serial.println("Wi-Fi Pass from SPIFFS: " + _wifi_password);
  Serial.println("Mist Maker on-time: " + String(_misterOnTime));
  Serial.println("Mist maker off-time: " + String(_misterOffTime));
  Serial.println("Average Humidity: " + String(_aHumidity));
  Serial.println("Average Temperature: " + String(_aTemperature));
  Serial.println("Average pH Level: " + String(_aPH));
  Serial.println();

  wifi_ssid = _wifi_ssid;
  wifi_password = _wifi_password;
  misterOnTime = _misterOnTime;
  misterOffTime = _misterOffTime;
  averageTemp = _aTemperature;
  averageHumid = _aHumidity;
  averagePH = _aPH;
  return true;
}



bool saveWiFiCredentialsSPIFFS() {
  doc["wifi_ssid"] = wifi_ssid;
  doc["wifi_pass"] = wifi_password;

  String tmp = "";
  if (!serializeJson(doc, tmp)) {
    Serial.println("save wifi credentials -> Error serializing JSON");
    return false;
  }
  if (!writeFile(SPIFFS, config_filename, tmp)) {
    Serial.println("save wifi credentials -> Error writing to file");
    return false;
  }

  return true;
}

bool readWiFiCredentialsSPIFFS(){

}

bool saveAverageParamsSPIFFS() {
  doc["humidity"] = averageHumid;
  doc["temperature"] = averageTemp;
  doc["pH"] = averagePH;

  String tmp = "";
  if (!serializeJson(doc, tmp)) {
    Serial.println("(save average params) -> Error serializing JSON");
    return false;
  }
  if (!writeFile(SPIFFS, config_filename, tmp)) {
    Serial.println("(save average params) -> Error writing to file");
    return false;
  }

  return true;
}
bool saveMisterConfigSPIFFS() {

  
  Serial.println("saveMisterConfigSPIFFS() function");
      Serial.printf("-------------------------- MISTER ON = %d------------------------------\n, misterOnTime");
      Serial.printf("-------------------------- MISTER OFF = %d------------------------------\n, misterOffTime");  

  doc["mister_on_time"] = String(misterOnTime);
  doc["mister_off_time"] = String(misterOffTime);

  String tmp = "";
  if (!serializeJson(doc, tmp)) {
    Serial.println("save wifi credentials -> Error serializing JSON");
    return false;
  }
  if (!writeFile(SPIFFS, config_filename, tmp)) {
    Serial.println("save wifi credentials -> Error writing to file");
    return false;
  }

  return true;
}

bool writeFile(fs::FS& fs, String filename, String message) {
  Serial.println("\nwriteFile -> Writing file: " + filename);

  File file = fs.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("writeFile -> failed to open file for writing");
    return false;
  }
  if (file.print(message)) {
    Serial.println("writeFile -> file written");
    return true;
  } else {
    Serial.println("writeFile -> write failed");
    return false;
  }
  file.close();
}

String readFile(fs::FS& fs, String filename) {
  Serial.println("readFile -> Reading file: " + filename);

  File file = fs.open(filename);
  if (!file || file.isDirectory()) {
    Serial.println("readFile -> failed to open file for reading");
    return "";
  }

  String fileText = "";
  while (file.available()) {
    fileText = file.readString();
  }

  file.close();
  return fileText;
}

// --------------------------- WEBSERVER HANDLERS ---------------------------- //


void handleRoot() {
  server.send(200, "text/html", SendHTML());
}


void handleSaveWiFiCredentials() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    const String _wifi_ssid = server.arg("ssid");
    const String _wifi_password = server.arg("password");

    Serial.println("SSID from WebServer: " + _wifi_ssid);
    Serial.println("SSID from WebServer: " + _wifi_password);

    wifi_ssid = _wifi_ssid;
    wifi_password = _wifi_password;

    if (saveWiFiCredentialsSPIFFS()) {
      Serial.println("saveWiFi -> saved Wi-Fi Credentials");
    }

    connectWiFi();
    server.send(200, "text/html", "Wi-Fi credentials saved successfully." + refreshPage(3));
  } else if (server.hasArg("ssid") && !server.hasArg("password")) {
    const String _wifi_ssid = server.arg("ssid");
    Serial.println("SSID from WebServer: " + _wifi_ssid);
    Serial.println("null");
    wifi_ssid = _wifi_ssid;
    wifi_password = "";
    if (saveWiFiCredentialsSPIFFS()) {
      Serial.println("saveWiFi -> saved Wi-Fi Credentials");
    }
    connectWiFi();
    server.send(200, "text/html", "Wi-Fi credentials saved successfully." + refreshPage(3));
  } else {
    server.send(200, "text/html", "Bad Request" + refreshPage(3));
  }
}

void handlesaveMisterConfig() {
  if (server.hasArg("onTime") && server.hasArg("offTime")) {

    
  Serial.println("handlesaveMisterConfig() function");
      Serial.printf("-------------------------- MISTER ON = %d------------------------------\n, misterOnTime");
      Serial.printf("-------------------------- MISTER OFF = %d------------------------------\n, misterOffTime"); 

    String MisterOnTime = server.arg("onTime");
    String MisterOffTime = server.arg("offTime");

    misterOnTime = MisterOnTime.toInt() * 60000;
    misterOffTime = MisterOffTime.toInt() * 60000;

    Serial.println(misterOnTime);
    Serial.println(misterOffTime);

    if (saveMisterConfigSPIFFS()) {
      Serial.println("saveMisterConfig -> saved Mister Configuration");
    }

    server.send(200, "text/html", "Mist Maker Configuration saved successfully." + refreshPage(3));
  } else {
    server.send(400, "text/html", "Bad Request" + refreshPage(3));
  }
}

// ----------------------------- WEBSERVER HTML ----------------------------- //

void handleWiFiConnect() {
  String webpage = "<!DOCTYPE html><html>";
  webpage += "<head>";
  webpage += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  webpage += "<title>AeroHouse AP: Wi-Fi Connect</title>";
  webpage += "</head>";
  webpage += "<body>";
  webpage += "<h1>AeroHouse Access Point Wi-Fi Connect</h1>";
  webpage += "<form action=\"/saveWiFiCredentials\" method=\"post\">";
  webpage += "Wi-Fi SSID:<br><input type=\"text\" name=\"ssid\"><br><br>";
  webpage += "Password:<br><input type=\"password\" name=\"password\"><br><br>";
  webpage += "<input type=\"submit\" value=\"Submit\">";
  webpage += "</form>";
  webpage += "</body></html>";

  server.send(200, "text/html", webpage);
}

void handleMisterConfig() {
  String webpage = "<!DOCTYPE html><html>";
  webpage += "<head>";
  webpage += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  webpage += "<title>AeroHouse AP: Mister Config</title>";
  webpage += "</head>";
  webpage += "<body>";
  webpage += "<h1>Mist Maker Configuration: Enter time in minutes</h1>";
  webpage += "<form action=\"/saveMisterConfg\" method=\"post\">";
  webpage += "Misters on-time:<br><input type=\"number\" name=\"onTime\"><br><br>";
  webpage += "Misters off-time::<br><input type=\"number\" name=\"offTime\"><br><br>";
  webpage += "<input type=\"submit\" value=\"Submit\">";
  webpage += "</form>";
  webpage += "</body></html>";

  server.send(200, "text/html", webpage);
}

String SendHTML() {
  String webpage;
  webpage += "<!DOCTYPE html><html>                                                       ";
  webpage += "<head>                                                                      ";
  webpage += "<meta name = \"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  webpage += "<title>AeroHouse AP: Monitoring</title>                                     ";
  webpage += "</head>                                                                     ";
  webpage += " <style>                                                                    ";
  webpage += " html {                                                                     ";
  webpage += "   font-family: Helvetica;                                                  ";
  webpage += "   display: inline-block;                                                   ";
  webpage += "   margin: 0px auto;                                                        ";
  webpage += "   text-align: center;                                                      ";
  webpage += " }                                                                          ";
  webpage += " body{                                                                      ";
  webpage += "   margin-top: 50px;                                                        ";
  webpage += " }                                                                          ";
  webpage += " h1 {                                                                       ";
  webpage += "   color: #444444;                                                          ";
  webpage += "   margin: 50px                                                             ";
  webpage += "   auto 30px;                                                               ";
  webpage += " }                                                                          ";
  webpage += " h3 {                                                                       ";
  webpage += "   color: #444444;                                                          ";
  webpage += "   margin-bottom: 50px;                                                     ";
  webpage += " }                                                                          ";
  webpage += " .button {                                                                  ";
  webpage += "   display: block;                                                          ";
  webpage += "   width: 80px;                                                             ";
  webpage += "   background-color: #3498db;border: none;color: white;padding: 13px 30px;  ";
  webpage += "   text-decoration: none;                                                   ";
  webpage += "   font-size: 25px;                                                         ";
  webpage += "   margin: 0px auto 35px;                                                   ";
  webpage += "   cursor: pointer;                                                         ";
  webpage += "   border-radius: 4px;                                                      ";
  webpage += " }                                                                          ";
  webpage += " p {                                                                        ";
  webpage += "   font-size: 14px;                                                         ";
  webpage += "   color: #888;                                                             ";
  webpage += "   margin-bottom: 10px;                                                     ";
  webpage += " }                                                                          ";
  webpage += " </style>                                                                   ";
  webpage += "<body><h1>AeroHouse Access Point Monitoring</h1>                            ";
  webpage += "<p>Temperature: " + String(temperatureValue) + " &#8451;</p>                ";
  webpage += "<p>Humidity: " + String(humidityValue) + " %</p>                            ";
  webpage += "<p>pH Level: " + String(pHValue) + " %</p>                                  ";

  webpage += "<p><a class=\"button\" href=\"/configureMister\">Mist Maker Config</a></p>  ";

  webpage += "<p><a class=\"button\" href=\"/connectWiFi\">Connect to Wi-Fi</a></p>       ";

  webpage += refreshPage(10);

  webpage += "</body></html>";

  return webpage;
}

String refreshPage(int time) {
  return "<script>setTimeout(function() { window.location.href = '/'; }," + String(time * 1000) + ");</script>";
}
