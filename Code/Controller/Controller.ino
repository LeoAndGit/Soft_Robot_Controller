
/* AT Control Board for soft robots
Version: 0.00
For: SoftRobotController-01
Author: Leo
Copyright 2019, github/LeoAndGit>
*/

#include "clsPCA9555.h"
//#include "Wire.h"

// Change X to the address you want
#define BOARD_ADDRESS "X"

#define LED0 PB0
#define LED1 PB1
#define DRVsleep PB9 // high enable

PCA9555 ioport_1 (0x20); // for sensor and DAC
PCA9555 ioport_2 (0x21); // for driver

String inputString = "";         // a string to hold incoming data

void setup() {
  afio_cfg_debug_ports(AFIO_DEBUG_NONE);// Stop the debug function
  // initialize the digital pin as an output.
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  ioport_1.begin();
  pinMode(DRVsleep, OUTPUT);

  // set first 14 pins to output
  for (uint8_t i = 0; i < 14; i++){
    ioport_1.pinMode(i, OUTPUT);
  }

  // set first 12 pins to output
  for (uint8_t i = 0; i < 12; i++){
    ioport_2.pinMode(i, OUTPUT);
  }

  /////////////////
  digitalWrite(DRVsleep,HIGH);// enable driver chip
  delay(10); //wait for driver

  for (uint8_t i = 0; i < 11; i++){
    ioport_2.digitalWrite(i, HIGH);
  }

  ////////////////

  // initialize serial.
  Serial.begin(115200); 
  Serial.setTimeout(1000); 

  /* wait for ADC */
  delay(10);

  digitalWrite(LED0,HIGH);// finish setup
  Serial.println("AT Control Board for soft robots");
  }

void loop() {
// dealing with AT command
if (Serial.available() > 0) {
  inputString = String(Serial.readStringUntil(10));//search for \n
  	if(inputString.endsWith(String('\r'))){
  		if (inputString=="AT\r"){
  			Serial.println("OK");
  		} 

  		else if(inputString=="AT+FREEALL\r"){
        // release all brakes

  			Serial.println("OK");
  		} 

  		else{
  			Serial.println("Error command");
  		}
  	}
  	else{
  		Serial.println("Error end check");
  	}
  
 }

}
