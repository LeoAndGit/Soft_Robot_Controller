
/* AT Control Board for Soft Robots
Version: 1.00
For: SoftRobotController-01
Author: Leo
Copyright 2019, github/LeoAndGit>
AD5761 driver code from:
https://github.com/hellange/AD5761/blob/master/AD5761.ino
*/

#include "clsPCA9555.h"
#include "SPI.h" // Include the Arduino SPI library

// Change X to the address you want
#define BOARD_ADDRESS "X"
#define BOARD_VERSION "V1.00"

// Define pins of MCU
#define LED0 PB0
#define LED1 PB1
#define DRVsleep PB9 // high enable

// Define pins of ioport_1 (for sensor and DAC)
#define S01 0
#define S02 1
#define S03 2
#define S04 3
#define S05 4
#define S06 5
#define S07 6
#define S08 7
#define S09 8
#define S10 9
#define S11 10
#define S12 11
#define DAC1 12
#define DAC2 13

// Define pins of ioport_2 (for driver)
#define R01 3
#define R02 2
#define R03 1
#define R04 0
#define R05 4
#define R06 5
#define R07 11
#define R08 10
#define R09 9
#define R10 8
#define R11 7
#define R12 6

// Input Shift Register Commands of AD57X1
#define CMD_NOP                 0x0
#define CMD_WR_TO_INPUT_REG     0x1
#define CMD_UPDATE_DAC_REG      0x2
#define CMD_WR_UPDATE_DAC_REG   0x3
#define CMD_WR_CTRL_REG         0x4
#define CMD_NOP_ALT_1           0x5
#define CMD_NOP_ALT_2           0x6
#define CMD_SW_DATA_RESET       0x7
#define CMD_RESERVED            0x8
#define CMD_DIS_DAISY_CHAIN     0x9
#define CMD_RD_INPUT_REG        0xA
#define CMD_RD_DAC_REG          0xB
#define CMD_RD_CTRL_REG         0xC
#define CMD_NOP_ALT_3           0xD
#define CMD_NOP_ALT_4           0xE
#define CMD_SW_FULL_RESET       0xF

PCA9555 ioport_1 (0x20); // for sensor and DAC
PCA9555 ioport_2 (0x21); // for driver

String inputString = "";         // a string to hold incoming data
int DAC1_Value = 0;
int DAC2_Value = 0;
byte ABP_data[2]; // ABP pressure sensor data buffer

void setup() {
  afio_cfg_debug_ports(AFIO_DEBUG_NONE);// Stop the debug function
  
  // Initialize digital pins as output
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(DRVsleep, OUTPUT);
  digitalWrite(DRVsleep,HIGH);// enable driver chip
  ioport_1.begin(); // Begin I2C

  // Set (IO Expander 1) first 14 pins to output
  for (uint8_t i = 0; i < 14; i++){
    ioport_1.pinMode(i, OUTPUT);
  }

  // Set (IO Expander 2) first 12 pins to output
  for (uint8_t i = 0; i < 12; i++){
    ioport_2.pinMode(i, OUTPUT);
  }

  // Set DAC & sensor ss pin to high
  for (uint8_t i = 0; i < 14; i++){
    ioport_1.digitalWrite(i, HIGH);
  }

  SPI.begin();  // Begin SPI hardware
  SPI.setClockDivider(SPI_CLOCK_DIV64);  // Slow down SPI clock
  
  // DAC Software reset
  ad5761r_write(CMD_SW_FULL_RESET, 0, DAC1);
  ad5761r_write(CMD_SW_FULL_RESET, 0, DAC2);

  // write control register
  // [23:21][20][19:16]   [15:11]  [10:9] 8   7   6   5 [4:3] [2:0]
  // |      |  |        |          |    |   |   |   |   |    |      |
  //   X X X  0  0 1 0 0  X X X X X  CV  OVR B2C ETS IRO  PV    RA
  //
  //   CV  : Clear voltage selection 00=zero, 01=midscale, 10,11=full scale
  //   OVR : 5% overrange 0=5% overrange disabled, 1=5% overrange enabled
  //   B2C : Bipolar range 0=DAC input for bipolar range is straight binary coded
  //                       1=DAC input for bipolar output range is twos complement code
  //   ETS : Thermal shutdown alert 0=does not power down when die temperature exceeds 150degC
  //                                1=powers down when die temperature exceeds 150degC
  //   IRO : Internal reference 0=off, 1=on
  //   PV  : Power up voltage 00=zero scale, 01=midscale, 10,11=full scale
  //   RA  : Output range
  //         000=-10 to +10
  //         001=0 to +10
  //         010=-5 to +5
  //         011=0 to +5
  //         100=-2.5 to +7.5
  //         101=-3 to +3
  //         110=0 to +16
  //         111=0 to +20
  
  // Initialize DAC control register
  ad5761r_write(CMD_WR_CTRL_REG, 0b0000000100001, DAC1);
  ad5761r_write(CMD_WR_CTRL_REG, 0b0000000100001, DAC2);

  // Initialize serial
  Serial.begin(115200); 
  Serial.setTimeout(1000); 

  delay(10);

  digitalWrite(LED0,HIGH);// finish setup
  }

void loop() {
// dealing with AT command
if (Serial.available() > 0) {
  inputString = String(Serial.readStringUntil(10));//search for \n
  	if(inputString.endsWith(String('\r'))){
      //
      // AT command test
      //
  		if (inputString=="AT\r"){
  			Serial.println("OK");
  		} 
      //
      // Reset system. All outputs become 0.
      //
  		else if(inputString=="AT+RESET\r"){
        ad5761r_write(CMD_WR_UPDATE_DAC_REG, 0x0000, DAC1);
        DAC1_Value = 0;
        ad5761r_write(CMD_WR_UPDATE_DAC_REG, 0x0000, DAC2);
        DAC2_Value = 0;
        for (uint8_t i = 0; i < 11; i++){
          ioport_2.digitalWrite(i, LOW);
        }
  			Serial.println("OK");
  		} 
      //
      // Reset driver. All outputs become 0.
      //
      else if(inputString=="AT+RESETR\r"){
        for (uint8_t i = 0; i < 11; i++){
          ioport_2.digitalWrite(i, LOW);
        }
        Serial.println("OK");
      } 
      //
      // Set DAC1
      //
      else if(inputString.substring(0,8)=="AT+DAC1="){
        String strDAC1_Value =  inputString.substring(8,11);
        // Add a 0 to HEX string for our DAC has only 12 bits but we need to write 16 bits
        strDAC1_Value += "0";
        // Turn HEX string into int
        DAC1_Value = strtol(strDAC1_Value.c_str(), NULL, 16);
        ad5761r_write(CMD_WR_UPDATE_DAC_REG, DAC1_Value, DAC1);
        Serial.println("OK");
      }
      //
      // Set DAC2
      //
      else if(inputString.substring(0,8)=="AT+DAC2="){
        String strDAC2_Value =  inputString.substring(8,11);
        // Add a 0 to HEX string for our DAC has only 12 bits but we need to write 16 bits
        strDAC2_Value += "0";
        // Turn HEX string into int
        DAC2_Value = strtol(strDAC2_Value.c_str(), NULL, 16);
        ad5761r_write(CMD_WR_UPDATE_DAC_REG, DAC2_Value, DAC2);
        Serial.println("OK");
      }
      //
      // Turn on driver
      //
      else if(inputString.substring(0,7)=="AT+RON="){
        String channel =  inputString.substring(7,9);
        switch (channel.toInt()){
          case 1:
            ioport_2.digitalWrite(R01, HIGH);
            break;
          case 2:
            ioport_2.digitalWrite(R02, HIGH);
            break;
          case 3:
            ioport_2.digitalWrite(R03, HIGH);
            break;
          case 4:
            ioport_2.digitalWrite(R04, HIGH);
            break;
          case 5:
            ioport_2.digitalWrite(R05, HIGH);
            break;
          case 6:
            ioport_2.digitalWrite(R06, HIGH);
            break;
          case 7:
            ioport_2.digitalWrite(R07, HIGH);
            break;
          case 8:
            ioport_2.digitalWrite(R08, HIGH);
            break;
          case 9:
            ioport_2.digitalWrite(R09, HIGH);
            break;
          case 10:
            ioport_2.digitalWrite(R10, HIGH);
            break;
          case 11:
            ioport_2.digitalWrite(R11, HIGH);
            break;
          case 12:
            ioport_2.digitalWrite(R12, HIGH);
            break;
          default:
            // statements
            Serial.println("Error channel");
            break;
        }
        Serial.println("OK");
      }
      //
      // Turn off driver
      //
      else if(inputString.substring(0,8)=="AT+ROFF="){
        String channel =  inputString.substring(8,10);
        switch (channel.toInt()){
          case 1:
            ioport_2.digitalWrite(R01, LOW);
            break;
          case 2:
            ioport_2.digitalWrite(R02, LOW);
            break;
          case 3:
            ioport_2.digitalWrite(R03, LOW);
            break;
          case 4:
            ioport_2.digitalWrite(R04, LOW);
            break;
          case 5:
            ioport_2.digitalWrite(R05, LOW);
            break;
          case 6:
            ioport_2.digitalWrite(R06, LOW);
            break;
          case 7:
            ioport_2.digitalWrite(R07, LOW);
            break;
          case 8:
            ioport_2.digitalWrite(R08, LOW);
            break;
          case 9:
            ioport_2.digitalWrite(R09, LOW);
            break;
          case 10:
            ioport_2.digitalWrite(R10, LOW);
            break;
          case 11:
            ioport_2.digitalWrite(R11, LOW);
            break;
          case 12:
            ioport_2.digitalWrite(R12, LOW);
            break;
          default:
            Serial.println("Error channel");
            // statements
            break;
        }
        Serial.println("OK");
      }
      //
      // Real data from ABP pressure sensor
      //
      else if(inputString.substring(0,9)=="AT+PREAD="){
        String channel =  inputString.substring(9,11);
        switch (channel.toInt()){
          case 1:
            ABP_read(S01);
            break;
          case 2:
            ABP_read(S02);
            break;
          case 3:
            ABP_read(S03);
            break;
          case 4:
            ABP_read(S04);
            break;
          case 5:
            ABP_read(S05);
            break;
          case 6:
            ABP_read(S06);
            break;
          case 7:
            ABP_read(S07);
            break;
          case 8:
            ABP_read(S08);
            break;
          case 9:
            ABP_read(S09);
            break;
          case 10:
            ABP_read(S10);
            break;
          case 11:
            ABP_read(S11);
            break;
          case 12:
            ABP_read(S12);
            break;
          default:
            Serial.println("Error channel");
            // statements
            break;
        }
        for (int i=0; i<2; i++){
          char format[2];
          sprintf(format, "%02X", ABP_data[i]);
          Serial.print(format);
        }
        Serial.println();
        Serial.println("OK");
      }
      //
      // Get board version
      //
      else if(inputString=="AT+VERSION\r"){
        Serial.print("AT Control Board for Soft Robots ");
        Serial.println(BOARD_VERSION);
        Serial.println("OK");
      } 
      //
      // Get address of this device
      //
      else if(inputString=="AT+ADDRESS\r"){
        Serial.println(BOARD_ADDRESS);
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

// This function is used to set DAC. 
// It needs to select which DAC you want to set
void ad5761r_write(uint8_t reg_addr_cmd,
            uint16_t reg_data,
            uint8_t DACss)
{
  SPI.setDataMode(SPI_MODE2);
  uint8_t data[3];
  //delay(1);
  ioport_1.digitalWrite(DACss, LOW);
  //delay(1);
  delayMicroseconds(100);

  data[0] = reg_addr_cmd;
  data[1] = (reg_data & 0xFF00) >> 8;
  data[2] = (reg_data & 0x00FF) >> 0;
  for (int i=0; i<3; i++)
  {
    SPI.transfer(data[i]);
  }
  //delay(1);
  ioport_1.digitalWrite(DACss, HIGH);
  //delay(1);
  delayMicroseconds(100);
}

// This function is used to read ABP pressure sensor. 
// It needs to select which sensor you want to read
void ABP_read(uint8_t ABPss)
{
  SPI.setDataMode(SPI_MODE0);
  ioport_1.digitalWrite(ABPss, LOW);
  delayMicroseconds(100);

  for (int i=0; i<2; i++)
  {
    ABP_data[i] = SPI.transfer(0);
  }
  //ABP_data_high = SPI.transfer(byte(0));
  //ABP_data_low = SPI.transfer(byte(0));
  ioport_1.digitalWrite(ABPss, HIGH);
  delayMicroseconds(100);
}
