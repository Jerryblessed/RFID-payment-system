/*
 * Author: donksy
 * Website: http://www.donskytech.com
 * Project: ARDUINO RFID DATABASE SYSTEM WITH ESP8266-ESP01
 * URL: https://www.donskytech.com/arduino-rfid-database-security-system-designing-the-project/
 */
 
#include <StreamUtils.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
 
SoftwareSerial mySerial(2, 3); // RX, TX
 
MFRC522 mfrc522(10, 9);   // Create Instance of MFRC522 mfrc522(SS_PIN, RST_PIN)
 
const int buzzerPin = 7; //buzzer attach to digital 7
const int ledPin = 4; //buzzer attach to digital 7
 
bool isRead = false;
bool isNewCard = false;
String tagContent = "";
String currentUID = "";
 
// Interval before we process the same RFID
int INTERVAL = 2000;
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
 
void setup()
{
   Serial.begin(115200);
  mySerial.begin(4800);
  mySerial.setTimeout(5000);
  // Setup the buzzer
  pinMode(buzzerPin, OUTPUT);
  // Setup the led
  pinMode(ledPin, OUTPUT);
 
  SPI.begin();
  mfrc522.PCD_Init();
 
  Serial.println("Detecting RFID Tags");
}
 
 
void loop()
{
  // Look for new cards
  if (mfrc522.PICC_IsNewCardPresent())
  {
    // Select one of the cards
    if (mfrc522.PICC_ReadCardSerial())
    {
      isRead = true;
 
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++)
      {
        tagContent.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        tagContent.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
 
      tagContent.toUpperCase();
    }
    if (isRead) {
      currentMillis = millis();
      // If current UID is not equal to current UID..meaning new card, then validate if it has access
      if (currentUID != tagContent) {
        currentUID =  tagContent;
        isNewCard = true;
      } else {
        // If it is the same RFID UID then we wait for interval to pass before we process the same RFID
        if (currentMillis - previousMillis >= INTERVAL) {
          isNewCard = true;
        } else {
          isNewCard = false;
        }
      }
      if (isNewCard) {
        if (tagContent != "") {
          // turn off LED if it is on for new card
          turnOffLED();
 
          previousMillis = currentMillis;
          //Send the RFID Uid code to the ESP-01 for validation
          Serial.print("Sending data to ESP-01 :: ");
          Serial.println(tagContent);
          //TODO Process read from serial here
          mySerial.println(tagContent);
          Serial.println("Waiting for response from ESP-01....");
 
          int iCtr = 0;
          while (!mySerial.available()) {
            iCtr++;
            if (iCtr >= 100)
              break;
            delay(50);
          }
          if (mySerial.available()) {
            bool isAuthorized = isUserAuthorized(tagContent);
 
            // If not authorized then sound the buzzer
            if (!isAuthorized) {
              playNotAuthorized();
            } else { //light up the Green LED
              turnOnLED();
            }
          }
          Serial.println("Finished processing response from ESP-01....");
        }
 
      }
 
    } else {
      Serial.println("No card details was read!");
    }
    tagContent = "";
    isNewCard = false;
  }
 
}
 
bool isUserAuthorized(String tagContent) {
  const size_t capacity = JSON_OBJECT_SIZE(1) + 30;
  DynamicJsonDocument doc(capacity);
 
  // Use during debugging
  //  ReadLoggingStream loggingStream(mySerial, Serial);
  //  DeserializationError error = deserializeJson(doc, loggingStream);
 
  // Use during actual running
  DeserializationError error = deserializeJson(doc, mySerial);
  if (error) {
    //    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return error.c_str();
  }
 
  bool   is_authorized = doc["is_authorized"] == "true";
  Serial.print("is_authorized :: ");
  Serial.println(is_authorized);
 
  return is_authorized;
}
 
void playNotAuthorized() {
  tone(buzzerPin, 2000);
  delay(500);
  noTone(7);
}
 
void turnOffLED() {
  digitalWrite(ledPin, LOW);
}
 
void turnOnLED() {
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
}