
/* AT Control Board for soft robots
Version: 0.00
For: SoftRobotController-01
Author: Leo
Copyright 2019, github/LeoAndGit>
AD5761 driver code:
https://github.com/hellange/AD5761/blob/master/AD5761.ino
*/

#include "clsPCA9555.h"
#include <SPI.h> // Include the Arduino SPI library

// Change X to the address you want
#define BOARD_ADDRESS "X"

#define LED0 PB0
#define LED1 PB1
#define DRVsleep PB9 // high enable

/* Input Shift Register Commands of AD57X1*/
#define CMD_NOP              0x0
#define CMD_WR_TO_INPUT_REG       0x1
#define CMD_UPDATE_DAC_REG        0x2
#define CMD_WR_UPDATE_DAC_REG     0x3
#define CMD_WR_CTRL_REG         0x4
#define CMD_NOP_ALT_1         0x5
#define CMD_NOP_ALT_2         0x6
#define CMD_SW_DATA_RESET       0x7
#define CMD_RESERVED          0x8
#define CMD_DIS_DAISY_CHAIN       0x9
#define CMD_RD_INPUT_REG        0xA
#define CMD_RD_DAC_REG          0xB
#define CMD_RD_CTRL_REG         0xC
#define CMD_NOP_ALT_3         0xD
#define CMD_NOP_ALT_4         0xE
#define CMD_SW_FULL_RESET       0xF

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

  ///////////////// testing driver
  digitalWrite(DRVsleep,HIGH);// enable driver chip
  delay(10); //wait for driver

  for (uint8_t i = 0; i < 11; i++){
    ioport_2.digitalWrite(i, HIGH);
  }

  ////////////////

  //////////////// testing DAC

  ioport_1.digitalWrite(13, HIGH);  // Set the SS pin HIGH
  SPI.begin();  // Begin SPI hardware
  //SPI.setClockDivider(SPI_CLOCK_DIV64);  // Slow down SPI clock
  SPI.setDataMode(SPI_MODE2);
  
  // software reset
  ad5761r_write(CMD_SW_FULL_RESET, 0);

  // write control register
  // [23:21][20][19:16]   [15:11]  [10:9] 8   7   6   5 [4:3] [2:0]
  // |      |  |        |          |    |   |   |   |   |    |      |
  //   X X X  0  0 1 0 0  X X X X X  CV  OVR B2C ETS IRO  PV    RA
  //
  //                                 CV  : Clear voltage selection 00=zero, 01=midscale, 10,11=full scale
  //                                 OVR : 5% overrange 0=5% overrange disabled, 1=5% overrange enabled
  //                                 B2C : Bipolar range 0=DAC input for bipolar range is straight binary coded
  //                                                     1=DAC input for bipolar output range is twos complement code
  //                                 ETS : Thermal shutdown alert 0=does not power down when die temperature exceeds 150degC
  //                                                              1=powers down when die temperature exceeds 150degC
  //                                 IRO : Internal reference 0=off, 1=on
  //                                 PV  : Power up voltage 00=zero scale, 01=midscale, 10,11=full scale
  //                                 RA  : Output range
  //                                       000=-10 to +10
  //                                       001=0 to +10
  //                                       010=-5 to +5
  //                                       011=0 to +5
  //                                       100=-2.5 to +7.5
  //                                       101=-3 to +3
  //                                       110=0 to +16
  //                                       111=0 to +20
  
  ad5761r_write(CMD_WR_CTRL_REG, 0b0000000100001);

  ad5761r_write(CMD_WR_UPDATE_DAC_REG, 0x7FF0);

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

void ad5761r_write(uint8_t reg_addr_cmd,
            uint16_t reg_data)
{
  uint8_t data[3];
  delay(1);
  ioport_1.digitalWrite(13, LOW);
  delay(1);

  data[0] = reg_addr_cmd;
  data[1] = (reg_data & 0xFF00) >> 8;
  data[2] = (reg_data & 0x00FF) >> 0;
  for (int i=0; i<3; i++)
  {
    SPI.transfer(data[i]);
  }
  delay(1);
  ioport_1.digitalWrite(13, HIGH);
  delay(1);
}