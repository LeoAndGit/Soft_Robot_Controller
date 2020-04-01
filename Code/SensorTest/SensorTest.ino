
/* Test program for pressure sensor
*/

#include "clsPCA9555.h"
#include "SPI.h" // Include the Arduino SPI library

// Change X to the address you want
#define BOARD_ADDRESS "X"

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
byte ABP_data[2];


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

  // Set DAC ss pin
  ioport_1.digitalWrite(DAC1, HIGH);  // Set the SS pin HIGH
  ioport_1.digitalWrite(DAC2, HIGH);  // Set the SS pin HIGH
  ioport_1.digitalWrite(S01, HIGH);  // Set the SS pin HIGH
  ioport_1.digitalWrite(S02, HIGH);  // Set the SS pin HIGH
  ioport_1.digitalWrite(S03, HIGH);  // Set the SS pin HIGH
  ioport_1.digitalWrite(S04, HIGH);  // Set the SS pin HIGH
  ioport_1.digitalWrite(S05, HIGH);  // Set the SS pin HIGH
  ioport_1.digitalWrite(S06, HIGH);  // Set the SS pin HIGH


  SPI.begin();  // Begin SPI hardware
  SPI.setClockDivider(SPI_CLOCK_DIV64);  // Slow down SPI clock
  //SPI.setDataMode(SPI_MODE2);

  // Initialize serial
  Serial.begin(115200); 
  Serial.setTimeout(1000); 

  delay(10);

  digitalWrite(LED0,HIGH);// finish setup
  }


void loop() {

	delay(1500);
	ABP_read(S02);

	for (int i=0; i<2; i++)
	  {
	  	char format[2];
	  	sprintf(format, "%02X", ABP_data[i]);
	    Serial.print(format);
	  }
	Serial.println();

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
