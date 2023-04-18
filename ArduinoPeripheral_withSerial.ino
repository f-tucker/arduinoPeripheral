/*
  Arduino BLE Peripheral with Serial Connection

  - This is part of a final year Industrial Group Project
  - Here, an Ardunio is used in a pair of custom Light Therapy glasses,
   to control a pair of LED's to turn on/off when the glasses are worn/not-worn
   and share changes in this with a custom app 'Blue Light' using BLE
  - Anotations made attempt to be general so are of more use to others

  - - - -

  --> code creates a BLE peripheral to hold LED state.
  --  Characterisitic and Service specified, then code advertises.
  --  Connect to the arduino using 'Blue Light' the companion ios app for this project
  --  While connected to Central, LED turns on/off depending on proximity sensor reading
  --  when LED state changes: LED state(0 or 1) and time(in seconds) printed to serial monitor, and LED state(0 or 1) writen to characteristic

  The circuit:

  - Arduino 33 BLE Sense
  - LED/resistors at Pin D12 or simply change to LEDBUILTIN of testing
  - pls refer to arduino documentation for suplying power.
  or, safely power with usb   

  - - - -

  - peripheral device is Arduino 33 BLE Sense
  - central device is smart phone running Blue Light
  - *requires serial connection*, this code was used for testing
  - peripheral can only connect to one central device at a time

  Note:

  - LED only turns on when device connected to central is connected. just to make easier timing on app for now
  - Maybe APDS can give -1 reading? 
  - Pin D12 is used, there are 3 references if a different pin is wanted or LEDBUILTIN for testing
  - for use in a circuit with a battery, without serial - just comment out serial references

*/

#include <ArduinoBLE.h>
#include <Arduino_APDS9960.h>

// BLE service,  randomly generated UUID for this project
BLEService LEDService("1037b18c-fc2d-4f8c-a9ba-6e45ed562713");

// BLE Characteristic, Central Devices can read Characteristics and Subscribe to Notifications if characteristic changes 
BLEUnsignedCharCharacteristic LEDChar("1037b18c-fc2d-4f8c-a9ba-6e45ed562713", BLERead | BLENotify); 

int oldLEDState = 0;  // variables for LED states 
int newLEDState = 0;


void setup() {

  // initialize serial communication
  Serial.begin(9600);   
  while (!Serial);

  // initialize proxitity sensor
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor!");
    delay(500); // to fill loop if no serial
  }

  // Initialise output pin
  pinMode(D12, OUTPUT);  


  // BLE initialization 
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  BLE.setLocalName("proximetyLED"); //<---------- set Local Name for BLE device, this will appear in advertising packets
  BLE.setAdvertisedService(LEDService); //<------ set the Service UUID which will be advertised
  LEDService.addCharacteristic(LEDChar); //<----- add Characteristic to Service
  BLE.addService(LEDService); // <--------------- add Service
  LEDChar.writeValue(oldLEDState); // <---------- set initial value of Characteristic as 0

  // start Advertising BLE
  // Advertising packets continuously transmitted until central device connects
  BLE.advertise();
  Serial.println("Bluetooth® device active, waiting for connections...");
}

void loop() {

  // wait for a BLE central
  BLEDevice central = BLE.central();
  
  // if a central is connected to the peripheral,  main body of code will run
  if (central) {
    Serial.print("Connected to central: ");

    // print the central's BT address:
    Serial.println(central.address());

    while (central.connected()) {
      if (APDS.proximityAvailable()){

        // read the proximity
        int proximity = APDS.readProximity();

        // determine if proximety is far or close, and update newLEDState variable accordingly
        if (proximity > 50) {         
          newLEDState = LOW;
        }
        else {
          newLEDState = HIGH;
        }  
    
        if (newLEDState != oldLEDState) { //<-------------------------------- if there is a new LED state, then:   
          digitalWrite(D12, newLEDState); //<------------------------ update value of LED output pin
          Serial.print("LED STATE = "); Serial.print(newLEDState); //<------| print new led state, and
          Serial.print(",  at time = "); Serial.println(millis()/1000); //<-| time in seconds to serial monitor
          LEDChar.writeValue(newLEDState); //<------------------------------- update BLE characteristic
          oldLEDState = newLEDState; //<------------------------------------- update oldLEDState variable
        }  
      }
    }
    // ----------------------------------------------- if disconnected from central, then:
    Serial.print("Disconnected from central: "); //<-| print an alert, and
    Serial.println(central.address()); //<-----------| central device's BT address to serial monitor
    newLEDState = LOW; oldLEDState = LOW; //<--------- update LED states variables to off
    digitalWrite(D12, LOW); //<--------------- update LED pin output to off
  }
}

