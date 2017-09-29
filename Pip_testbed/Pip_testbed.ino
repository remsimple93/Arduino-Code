// Creator: Saeyoung Kim
// reading from pH (OCP) and DO (amperometry) readout circuit and writing to LCD and microSD card
// Last updated: 2017.09.25

// including libraries
#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"


// Variable declaration
int pHreadout = A0;
int DOreadout = A1;
int Tempreadout = A2;
float pHvoltage;
float DOvoltage;
float Tempvoltage;
int ID = 1;                 // number of data entries in one file
const int chipSelect = 10;
File dataFile;
float temperatureC;         // Temperature in Celcius
String dataString;          // data entry for experiments
String CurrentTime;         // for making file names
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"}; // for RTC to refer to
long elapsedTime;           // to be used as timer during experiments
long startTime;
String fileName;
char pHbuff[9];
char DObuff[9];
char tempbuff[7];
String currentMonth;
String currentDay;
String currentHour;
String currentMin;
String currentSec;

// initialize library with number of interface pins
LiquidCrystal lcd(8, 9, 2, 3, 4, 5);

// create an RTC object
RTC_DS1307 rtc;

// -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void setup() {

  // writing to LCD ----------------------------------------------------------------------------

    // begin Serial Monitoring
    Serial.begin(57600);
  
    // setup LCD's number of columns and rows
    lcd.begin(16, 2);

    // turn on LCD
    lcd.display();

    // It's alive!
    lcd.clear();
    lcd.print(F("By jolly..."));
    lcd.setCursor(0, 1);
    lcd.print(F("I have awoken!"));
    delay(2000);

    // LCD status display
    lcd.clear();
    lcd.print(F("Initializing..."));
    delay(1000);

    // RTC check
    Serial.println(F("Initializing RTC..."));
    lcd.clear();
    lcd.print(F("Checking RTC"));
    delay(1000);


  // initialize RTC ------------------------------------------------------------------------------
    rtc.begin();
    
    // look for RTC, if not notify me on Serial Monitor
    if (! rtc.begin()) {
        Serial.println(F("RTC NOT found."));
       
        // RTC check
        lcd.clear();
        lcd.print(F("RTC NOT found."));
        delay(1000);
        while (1);
    }

    // if RTC is not running, set the time and start running it
    if (! rtc.isrunning()) {
    Serial.println(F("RTC is NOT running!"));
    lcd.clear();
    lcd.print(F("RTC NOT running."));
    delay(1000);
    
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    Serial.println(F("RTC adjusted."));
    lcd.clear();
    lcd.print(F("RTC adjusted."));
    delay(1000);
    }

    Serial.println(F("RTC initialization... DONE."));
    lcd.clear();
    lcd.print(F("RTC initialization... DONE."));
    delay(1000);

    // define DateTime and CurrentTime string variable (call entire date and time atm, we'll cut it up later)
    DateTime now = rtc.now();

    // DS1307 output formatting fix
    if ( now.month() < 10 ){
      currentMonth = "0" + String(now.month());
    }
    else{
      currentMonth = String(now.month());
    }

    if(now.day() < 10){
      currentDay = "0" + String(now.day());
    }
    else{
      currentDay = String(now.day());
    }

    if(now.hour() < 10){
      currentHour = "0" + String(now.hour());
    }
    else{
      currentHour = String(now.hour());
    }

    if(now.minute() < 10){
      currentMin = "0" + String(now.minute());
    }
    else{
      currentMin = String(now.minute());
    }

    if(now.second() < 10){
      currentSec = "0" + String(now.second());
    }    
    else{
      currentSec = String(now.second());
    }

    CurrentTime = String(now.year()) + "/" + currentMonth + "/" + currentDay + " " + currentHour + ":" + currentMin + ":" + currentSec;

    // Print current time on Serial Monitor for debugging purposes   
    Serial.println("Current time is " + CurrentTime);

  // writing to microSD module --------------------------------------------------------------------
    
    pinMode (chipSelect, OUTPUT);

    // check SDcard
    lcd.clear();
    lcd.print(F("Checking SDcard"));
    delay(1000);

    // initialize microSD card module
    Serial.println(F("Initializing SD card..."));

    // check if microSD is running, if not notify me on Serial Monitor
    if (!SD.begin(chipSelect)) { 
      Serial.println(F("SD card initialization failed."));
     
      // print to LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("SD card initialization..."));
      lcd.setCursor(0, 1);
      lcd.print(F("FAIL"));
      delay(1000);      
      return;
      
    }
   
    // SDcard check done!
    Serial.println(F("SD card initialization... DONE."));
    lcd.clear();
    lcd.print(F("SD card ready!"));
    delay(1000);
    
    // define String fileName variable and print to SM
    fileName = currentMonth + currentDay + currentHour + currentMin;
    Serial.println("File name is " + fileName);

    // print to LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Initialization..."));
    lcd.setCursor(0, 1);
    lcd.print(F("DONE"));
    delay(1000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Here we go!"));
    delay(1000);

    // Initial time when starting experiment
    startTime = now.unixtime() + 4;
    Serial.println("Starting unixtime: " + String(startTime));

    // open Data File I will be working with and write header for identification
    File dataFile = SD.open(fileName + ".csv", FILE_WRITE);

    // if file was succesfully opened write data string
    if (dataFile) {

      dataFile.println(CurrentTime + "," + "Time (s)" + "," + "pH readout V" + "," + "DO readout V" + "," + "Temp (C)");
      dataFile.close();
      Serial.println(CurrentTime + " written to file as header.");
      
    }
    
    // if file FAILED to open
    else {
      Serial.println(F("File FAILED to open."));
    }
  
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop() {

  // read from analog pins and convert ---------------------------------------------------------------
    // get voltage reading from A0 (pH), A1 (DO), and A2 (Temp.)
    int pHreadout = analogRead(A0);
    int DOreadout = analogRead(A1);
    int Tempreadout = analogRead(A2);

    // convert reading to voltage
    float pHvoltage = pHreadout * 5.0/1024;
    float DOvoltage = DOreadout * 5.0/1024;
    float Tempvoltage = Tempreadout * 5.0/1024;

    // convert voltage to temperature, converting from 10 mV per degree with 500 mV offset to temperature
    float temperatureC = (Tempvoltage - 0.5) * 100;

    // print to serial monitor for troubleshooting
    // Serial.print(String("pH Voltage: ") + String(pHvoltage) + String(" V") + String(", ") + String("DO voltage: ") + String(DOvoltage) + String(" V") + String(", ") + String("Temp: " + String(temperatureC)) + String(" C") + String(", "));

  // writing to LCD ----------------------------------------------------------------------------------

    // clear LCD
    lcd.clear();

    // put cursor on first row first column
    lcd.setCursor(0, 0);

    // print pHvoltage on LCD
    lcd.print(F("pH V: "));
    lcd.print(pHvoltage, 6);
    lcd.print(F(" V"));

    // put cursor on second row first column
    lcd.setCursor(0, 1);

    // print DOvoltage on LCD
    lcd.print(F("DO V: "));
    lcd.print(DOvoltage, 6);
    lcd.print(F(" V"));

  // writing to microSD card module -------------------------------------------------------------------

    // convert float variables to strings using dtostrf
    dtostrf(pHvoltage, 8, 6, pHbuff);
    dtostrf(DOvoltage, 8, 6, DObuff);
    dtostrf(temperatureC, 6, 2, tempbuff);

    // calculate elapsed time since start of power
    DateTime now = rtc.now();
    elapsedTime = now.unixtime() - startTime;

    // create data string to write to SD card
    dataString = String(ID) + ", " + String(elapsedTime) + ", " + pHbuff + ", " + DObuff + "," + tempbuff;
    
    // open Data File I will be working with
    File dataFile = SD.open(fileName + ".csv", FILE_WRITE);

    // if file was succesfully opened write data string
    if (dataFile) {

      dataFile.println(dataString);
      dataFile.close();
      Serial.println(dataString);
      
    }
    
    // if file FAILED to open
    else {
      Serial.println(F("File FAILED to open."));
    }

    // increment ID number
    ID++;

  delay(2000);
}
