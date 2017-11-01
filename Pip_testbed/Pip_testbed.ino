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
int pHtotal = 0;
int DOreadout = A1;
int DOtotal = 0;
int Tempreadout = A2;
int Temptotal = 0;
int i = 0;                           // counter for quick successive analog input readings
float pHvoltage;
float DOvoltage;
float Tempvoltage;
int ID = 1;                          // number of data entries in one file
const int chipSelect = 10;
File dataFile;
float temperatureC;                  // Temperature in Celcius
String dataString;                   // data entry for experiments
String CurrentTime;                  // for making file names
long elapsedTime;           // legacy code
float con_elapsedMillis;             // elapsedMillis converted to seconds
unsigned long startTime;             // legacy code
unsigned long startMillis;           // starting millisecond timestamp
unsigned long elapsedMillis;         // elapsed millisecond for timestamping
unsigned long previousMillis;        // previous millisecond for deciding if or not to proceed with next void loop
unsigned long currentMillis;         // current millisecond for deciding if or not to proceed with next void loop
unsigned long samplingTime = 500;   // sampling time in milliseconds, currently set to 1 second, keep it above 330
String fileName;
char pHbuff[6];
char DObuff[6];
char tempbuff[6];
char timebuff[8];
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
    lcd.print(F("RTC ready!"));
    delay(1000);

    // define DateTime and CurrentTime string variable (call entire date and time atm, we'll cut it up later)
    DateTime now = rtc.now();

    // DS1307 output formatting fix --------------------------------------------------------------------
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
    
    // define String fileName variable and print to SM and LCD
    fileName = currentMonth + currentDay + currentHour + currentMin;
    Serial.println("File name is " + fileName);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Filename:"));
    lcd.setCursor(0, 1);
    lcd.print(fileName);
    delay(5000);

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
    startTime = now.unixtime() + 9;
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

    // timestamp start 
    startMillis = millis();
    
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop() {

  // timestamp for sampling time
  previousMillis = millis();

  // read from analog pins and convert ---------------------------------------------------------------
    // get voltage reading from A0 (pH), A1 (DO), and A2 (Temp.)
    // initial ping
    analogRead(A0);
    delay(10);

    // get 10 readings of pHreadout  ---------------------------------------------------------------
    for(i = 0; i < 10; i++)
      {
        pHreadout = analogRead(A0);
        pHtotal = pHtotal + pHreadout; // add new pHreadout to old pHreadout, do this from count 0 to 9
        delay(10);
      }

      pHreadout = pHtotal / 10; // avg of 10 readings
      pHtotal = 0;
      i = 0;                    // reinitialize i as 0 

    // initial ping    
    analogRead(A1); // initial ping
    delay(10);

    // get 10 readings of DOreadout  ---------------------------------------------------------------
    for(i = 0; i < 10; i++)
      {
        DOreadout = analogRead(A1);
        DOtotal = DOtotal + DOreadout; // add new DOreadout to old DOreadout, do this from count 0 to 9
        delay(10);
      }

      DOreadout = DOtotal / 10; // avg of 10 readings
      DOtotal = 0;
      i = 0;                    // reinitialize i as 0 

    // initial ping  
    analogRead(A2);
    delay(10);

    // get 10 readings of Tempreadout  ---------------------------------------------------------------
    for(i = 0; i < 10; i++){
        Tempreadout = analogRead(A2);
        Temptotal = Temptotal + Tempreadout; // add new Tempreadout to old Tempreadout, do this from count 0 to 9
        delay(10);
      }

      Tempreadout = Temptotal / 10; // avg of 10 readings
      Temptotal = 0;
      i = 0;                        // reinitialize i as 0 

    // convert reading to voltage
    float pHvoltage = pHreadout * 5.0/1024;
    float DOvoltage = DOreadout * 5.0/1024;
    float Tempvoltage = Tempreadout * 5.0/1024;

    // convert voltage to temperature, converting from 10 mV per degree with 500 mV offset to temperature
    float temperatureC = (Tempvoltage - 0.5) * 100;

  // writing to LCD ----------------------------------------------------------------------------------

    // clear LCD
    lcd.clear();

    // put cursor on first row first column
    lcd.setCursor(0, 0);

    // print pHvoltage on LCD
    lcd.print(F("pH: "));
    lcd.print(pHvoltage, 3);
    lcd.print(F(" V"));

    // put cursor on second row first column
    lcd.setCursor(0, 1);

    // print DOvoltage on LCD
    lcd.print(F("DO: "));
    lcd.print(DOvoltage, 3);
    lcd.print(F(" V"));

  // writing to microSD card module -------------------------------------------------------------------

    // convert sensor readings float variables to strings using dtostrf
    dtostrf(pHvoltage, 5, 3, pHbuff);
    dtostrf(DOvoltage, 5, 3, DObuff);
    dtostrf(temperatureC, 5, 1, tempbuff);

    // calculate elapsed time since start of power, legacy code
    // DateTime now = rtc.now();
    // elapsedTime = now.unixtime() - startTime;

    // calculate elapsed millisecond since startMillis timestamp
    elapsedMillis = millis() - startMillis;
    con_elapsedMillis = elapsedMillis * 0.001; // multiply elapsed millisecond with 0.001 to convert to seconds

    // convert elapsed time float variables to strings using dtostrf
    dtostrf(con_elapsedMillis, 7, 3, timebuff);

    // print seconds on LCD
    lcd.setCursor(12, 0);
    lcd.print(con_elapsedMillis, 0);
    lcd.setCursor(12, 1);
    lcd.print(F("s"));
    
    // create data string to write to SD card
    dataString = String(ID) + ", " + timebuff + ", " + pHbuff + ", " + DObuff + "," + tempbuff;
    
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

  // wait until samplingTime is acheived, then move onto next void loop
  currentMillis = millis();
  while(currentMillis - previousMillis < samplingTime){
    delay(10);
    currentMillis = millis();
  }
  
}
