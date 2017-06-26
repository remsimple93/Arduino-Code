// Creator: Saeyoung Kim
// reading TMP36 (voltage outpute temperature sensor),  writing to 16x2 LCD and microSD module
// Last updated: 2017.06.26

// including libraries
#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>

// Variable declaration
int sensorPin = A0;
int ID = 1;
const int chipSelect = 10;
File dataFile;
float readVoltage;
float temperatureC;
String dataString;

// initialize library with number of interface pins
LiquidCrystal lcd(8, 9, 2, 3, 4, 5);

// ---------------------------------------------------------------

void setup() {

  // writing to LCD

    // begin Serial Monitoring
    Serial.begin(9600);
  
    // setup LCD's number of columns and rows
    lcd.begin(16, 2);

    // turn on LCD
    lcd.display();

    // clear LCD
    lcd.clear();

    // LCD status display
    lcd.print("LCD : Ready");
    lcd.setCursor(0, 1);
    lcd.print("Commencing ...");

  delay(2000);

  // writing to microSD module

    // clear LCD
    lcd.clear();

    //
    pinMode (chipSelect, OUTPUT);

    // initialize microSD card module
    Serial.println("Initializing SD card...");
    
    if (!SD.begin(chipSelect)) {
    
      Serial.println("Initialization failed.");
     
      // print to LCD
      lcd.setCursor(0, 0);
      lcd.print("Initialization...");
      lcd.setCursor(0, 1);
      lcd.print("FAIL");
      return;
      
    }

    Serial.println("Initialization... DONE.");
    
    // print to LCD
    lcd.setCursor(0, 0);
    lcd.print("Initialization...");
    lcd.setCursor(0, 1);
    lcd.print("DONE");

/*
  
    // creat data string to write to SD card ------
    String dataString = String(ID) + ", " + String(readVoltage) + ", " + String(temperatureC);
    String header = String("ID") + ", " + String("Voltage (V)") + ", " + String("Temperature(Celcius)");

    // Create and open txt file
    File dataFile = SD.open("LOG.csv", FILE_WRITE);

    // if file was succesfully opened
    if (dataFile) {
      Serial.println("File was opened.");
      dataFile.println(header);
      dataFile.close();
      Serial.println(header);
    }

*/

  delay(2000);
  
}

// ---------------------------------------------------------------

void loop() {

  // writing to LCD ------

    // clear LCD
    lcd.clear();

    // get voltage reading from temperature sensor
    int reading = analogRead(sensorPin);

    // convert reading to voltage
    float readVoltage = reading * 5.0/1024;

    // convert voltage to temperature, converting from 10 mV per degree with 500 mV offset to temperature
    float temperatureC = (readVoltage - 0.5) * 100;

    // print to serial monitor for troubleshooting
    Serial.print(readVoltage);
    Serial.println(" volts");
    Serial.print(temperatureC);
    Serial.println(" degrees C");

    // put cursor on first row first column
    lcd.setCursor(0, 0);

    // print voltage on LCD
    lcd.print("V:  ");
    lcd.print(readVoltage);
    lcd.print(" volts");

    // put cursor on second row first column
    lcd.setCursor(0, 1);

    // print temperature on LCD
    lcd.print("T: ");
    lcd.print(temperatureC);
    lcd.print(" deg C");

  // writing to microSD card module ------

    // Create and open CSV file
    File dataFile = SD.open("LOG.csv", FILE_WRITE);

    // create data string to write to SD card ------
    String dataString = String(ID) + ", " + String(readVoltage) + ", " + String(temperatureC);
    String header = String("ID") + ", " + String("Voltage (V)") + ", " + String("Temperature(Celcius)");  // code not used atm

    // if file was succesfully opened
    if (dataFile) {

      dataFile.println(dataString);
      Serial.println(dataString);
      dataFile.close();
      
    }
    
    // if file FAILED to open
    else {
      Serial.println("File FAILED to open.");
    }

    // increment ID number
    ID++;

  delay(1000);
}
