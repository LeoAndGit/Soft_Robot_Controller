[![Release](https://img.shields.io/badge/Release-Ver0.1.0-blue.svg)](https://github.com/LeoAndGit/Soft_Robot_Controller/releases)

Soft_Robot_Controller
===========================
This a board for soft robots. It has 12 channels of 24V half-bridge output to control electromagnetic valves, 12 pressure sensors and 2 DACs to provide 0V-10V analog signals. 

****
Author: Leo

Commercial use is not allowed, if not consulted me.
****

Control board
---------------------------
This board is a four layer PCB. It needs both USB (for 3.3V power) and a 24V power adapter(GST60A24-P1J).

Use this board is really simple. Built in CH340 USB to serial converter makes it possible to program your STM controller without any other device. To upload new program, just change the switch on left side to BOOT mode and use serial in Arduino IDE. Or just use AT commands to control it.

***************************

BUGs on SoftRobotController-01:
- The location of IDC connector is not very good.
- Magnetic bead should be better between STM and analog part to reduce noise. I use a 0ohm resister to replace it in version 01 PCB.

***************************

- PCB 
<img src="/Image/PCB01.png" width="450px" />

- Photo of board (not finished yet)
<img src="/Image/board01.jpg" width="450px" />

Arduino code
---------------------------
About the whole control system, you can get some information on my another project: https://github.com/LeoAndGit/Arduino_Simple_AT_Command This project uses the same architecture to communicate and control.

Arduino STM32 library: https://github.com/rogerclarkmelbourne/Arduino_STM32

You will still need this PCA9555 library modified by myself: https://github.com/LeoAndGit/PCA9555

***************************

V0.1.0 code feature:
- Support control every independent half-bridge output
- Support 2 channels of analog output
- Support multiple boards working together by different addresses
- Not support pressure sensors yet

A red LED shows 3.3V logic power is online and a green LED tells you this MCU is working now. 

How to control this board: send AT commands to it with baud of 115200. 

- **Send**: AT\r\n  
  **Reply**: OK\r\n  
  **Description**: for testing 


- **Send**: AT+VERSION\r\n  
  **Reply:** VERSION\r\n OK\r\n  
  **Description**: get version


- **Send**: AT+ADDRESS\r\n  
  **Reply**: ADDRESS\r\n OK\r\n  
  **Description**: get address. This means when you connect this board to your PC, you can know which board is talking to.


- **Send**: AT+RESET\r\n  
  **Reply**: OK\r\n  
  **Description**: reset all voltage to 0v and all half-bridges are set to low output


- **Send**: AT+RESETR\r\n  
  **Reply**: OK\r\n  
  **Description**: all half-bridges are set to low output


- **Send**: AT+DAC1=xxx\r\n  
  **Reply**: OK\r\n  
  **Description**: set output voltage of DAC1. This xxx is a hexadecimal number. With this setting of DAC, it means 000 will output 0V and FFF will output 10V.


- **Send**: AT+DAC2=xxx\r\n  
  **Reply**: OK\r\n  
  **Description**: set output voltage of DAC2. Same with DAC1.


- **Send**: AT+RON=xx\r\n  
  **Reply**: OK\r\n  
  **Description**: let half-bridge output high. xx is which port you want to select. xx=01 means port R1 on board.


- **Send**: AT+ROFF=xx\r\n  
  **Reply**: OK\r\n  
  **Description**: let half-bridge output low. Same with RON command.