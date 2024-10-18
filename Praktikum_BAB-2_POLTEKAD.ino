#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "PRIMA TP LINK LANTAI 2"
#define WIFI_PASSWORD "DUAPRIMA2020"

/* 2. Define the API Key */
#define API_KEY "AIzaSyCUIn8Vef9vI7y_989f_j7RA1ihBOZilvQ"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "hidroponik-iot"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "zaidanr4f1@gmail.com"
#define USER_PASSWORD "Zaidan12345"

//reset board
void(* di_reset) (void) = 0;

//SUHU UDARA
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN D4     // Digital pin connected to the DHT sensor 

#define DHTTYPE    DHT11     // DHT 11

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

float suhuUdara;
float kelembapanUdara;


// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
int count = 0;
int count1 = 0;

// The Firestore payload upload callback function
void fcsUploadCallback(CFS_UploadStatusInfo info)
{
    if (info.status == fb_esp_cfs_upload_status_init)
    {
        Serial.printf("\nUploading data (%d)...\n", info.size);
    }
    else if (info.status == fb_esp_cfs_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s\n", (int)info.progress, "%");
    }
    else if (info.status == fb_esp_cfs_upload_status_complete)
    {
        Serial.println("Upload completed ");
    }
    else if (info.status == fb_esp_cfs_upload_status_process_response)
    {
        Serial.print("Processing the response... ");
    }
    else if (info.status == fb_esp_cfs_upload_status_error)
    {
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void setup()
{

    Serial.begin(57600);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

#if defined(ESP8266)
    // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
#endif
    
    // Limit the size of response payload to be collected in FirebaseData
    fbdo.setResponseSize(2048);

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);

    // For sending payload callback
    // config.cfs.upload_callback = fcsUploadCallback;

  timeClient.begin();

  timeClient.setTimeOffset(0);


  //SUHU UDARA
  //Serial.begin(57600);
  // Initialize device.
  dht.begin();
  //Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
/*  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
*/  // Set delay between sensor readings based on sensor details.
  //delayMS = sensor.min_delay / 1000;

}
void loop(void){
  //SUHU UDARA
  // Delay between measurements.
  //delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
   Serial.println(F("Error reading temperature!")); 
  }
  else {
/*    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C")); */
    suhuUdara = event.temperature;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));  
  }
  else {
    //Serial.print(F("Humidity: "));
    //Serial.print(event.relative_humidity);
    //Serial.println(F("%"));
    kelembapanUdara = event.relative_humidity;
  }/*
  Serial.println();
*/




/////////firebase
    //current time 
  timeClient.update();

  time_t epochTime = timeClient.getEpochTime();

  //Serial.println(timeClient.getFormattedTime());

  //get days
  ////String weekDay = weekDays[timeClient.getDay()];
  //Serial.print("Hari: ");
  //Serial.println(weekDay);

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  int currentYear = ptm->tm_year+1900;

  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay) + "T" + timeClient.getFormattedTime() + "Z";
/*  Serial.print("Current date: ");
  Serial.println(currentDate);
*/
  //delay(1000);

  
if (Firebase.ready() && (millis() - dataMillis > 500 || dataMillis == 0))
    {
        dataMillis = millis();

        FirebaseJson content;
        String documentPathData = "hydroponic/data";

        content.set("fields/airtemp/doubleValue", suhuUdara);
        content.set("fields/airhumidity/doubleValue", kelembapanUdara);
        //content.set("fields/watervolume/doubleValue", tinggiAir);

        String doc_path = "projects/";
        doc_path += FIREBASE_PROJECT_ID;
        doc_path += "/databases/(default)/documents/hydroponic/airtemp"; // coll_id and doc_id are your collection id and document id
        doc_path += "/databases/(default)/documents/hydroponic/airhumidity"; // coll_id and doc_id are your collection id and document id

        // timestamp
        content.set("fields/time/timestampValue", currentDate); // RFC3339 UTC "Zulu" format
        
        if(count1 == 0){
              if (Firebase.Firestore.deleteDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPathData.c_str()))
                Serial.printf("ok deleted\n%s\n\n", fbdo.payload().c_str());
              else
                Serial.println(fbdo.errorReason());
            }

        if(count1 != 0){
          if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPathData.c_str(), content.raw()))
              Serial.printf("ok stored\n%s\n\n", fbdo.payload().c_str());
          else
              Serial.println(fbdo.errorReason()); 
        }
        
        count1++;
        delay(4300);

        while(count1 == 2){  
          count1 = 0;
        }



/////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
/*
      if (Firebase.ready() && (millis() - dataMillis > 3000 || dataMillis == 0))
    {
        dataMillis = millis();

        FirebaseJson content;

        // We will create the nested document in the parent path "a0/b0/c0
        // a0 is the collection id, b0 is the document id in collection a0 and c0 is the collection id in the document b0.
        // and d? is the document id in the document collection id c0 which we will create.
        String documentPathAirTemp = "hydroponic/airtemp" + String(count);
        
        // If the document path contains space e.g. "a b c/d e f"
        // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"

        // double
        content.set("fields/temp/doubleValue", 32.5);

        String doc_path = "projects/";
        doc_path += FIREBASE_PROJECT_ID;
        doc_path += "/databases/(default)/documents/coll_id/doc_id"; // coll_id and doc_id are your collection id and document id

        // timestamp
        content.set("fields/time/timestampValue", currentDate); // RFC3339 UTC "Zulu" format


        // map
       // content.set("fields/myMap/mapValue/fields/name/stringValue", "wrench");
       // content.set("fields/myMap/mapValue/fields/mass/stringValue", "1.3kg");
       // content.set("fields/myMap/mapValue/fields/count/integerValue", "3");

        count++;
        
        //delete
                      if (Firebase.Firestore.deleteDocument(&fbdo, FIREBASE_PROJECT_ID, "" , documentPathAirTemp.c_str()))
                Serial.printf("ok deleted\n%s\n\n", fbdo.payload().c_str());
              else
                Serial.println(fbdo.errorReason());


            

       //create
        Serial.print("Create a document... ");

        
        

        
        if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" , documentPathAirTemp.c_str(), content.raw()))
            Serial.printf("ok stored\n%s\n\n", fbdo.payload().c_str());
        else
            Serial.println(fbdo.errorReason()); 

        
           
            
        while(count == 4){  
          count = 0;
        }
        
    }

*/
                
    }
    else if(!Firebase.ready()){
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      Firebase.begin(&config, &auth);
      Firebase.reconnectWiFi(true);
    }
        ///////////
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Firebase.ready() should be called repeatedly to handle authentication tasks.

    

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

    
}
