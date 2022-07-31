/*
 * Author: donksy
 * Website: http://www.donskytech.com
 * Project: ARDUINO RFID DATABASE SYSTEM WITH ESP8266-ESP01
 * URL: https://www.donskytech.com/arduino-rfid-database-security-system-designing-the-project/
 */
 
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
 
// NOTE:  CHANGE THIS TO YOUR WIFI SSID AND PASSWORD
const char* ssid     = "";
const char* password = "";
 
//#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif
 
void setup() {
  Serial.begin(4800);
  delay(10);
 
  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
const byte numChars = 20;
char receivedChars[numChars];   // an array to store the received data
 
boolean newData = false;
 
void loop() {
    recvWithEndMarker();
    processRFIDCode();
}
void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
    
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();
 
        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}
 
void processRFIDCode() {
    if (newData == true) {
        DPRINT("This just in ... ");
        DPRINTLN(receivedChars);
        newData = false;
        isUserAuthorized(receivedChars);
    }
}
 
 
 
void isUserAuthorized(String rfIdCode){
    // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
 
    WiFiClient client;
 
    HTTPClient http;
 
    // NOTE:  CHANGE THIS TO THE IP ADDRESS WHERE YOUR APPLICATION IS RUNNING
    String url = "http://192.168.137.1:8080/student/isauthorized?rf_id_code=";
    String encodedRFIDCode = urlencode(rfIdCode);
    url+=encodedRFIDCode;
    DPRINT("url :: ");
    DPRINTLN(url);
 
    http.setTimeout(20000);
    if (http.begin(client, url)) {  // HTTP
 
      // start connection and send HTTP header
      int httpCode = http.GET();
      DPRINT("httpCode :: ");
      DPRINTLN(httpCode);
 
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
 
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          DPRINTLN(payload);
          sendDataBackToArduino(payload);
 
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
 
      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }
}
 
String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
     
}
 
void sendDataBackToArduino(String payload){
  const size_t capacity = JSON_OBJECT_SIZE(1) + 30;
  DynamicJsonDocument doc(capacity);
    // Parse JSON object
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  serializeJson(doc, Serial);
}