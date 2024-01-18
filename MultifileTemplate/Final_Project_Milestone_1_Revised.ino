/*A multifile project code template
  A template for the Milestone 1 project code that uses multiple files
  for modularity. The compiler first loads the principal file 
  (the one with the same name as the folder) and then loads 
  the others in alphabetical order. Variables defined in an 
  earlier file will be visible to code in a file that is loaded 
  later, but not vice-versa. 

  Calls functions in files:
  MotorFunctions.ino

  written for the MSP432401 board
  Author: Deborah Walter
  Last revised: 11/28/2023

***** Hardware Connections: *****
     start button       P3.0
     playstation connections
     brown wire         P1.7 
     orange wire        P1.6 
     yellow wire        P2.3
     blue wire          P6.7
*/

// Load Libraries used
#include "PS2X_lib.h"  //for v1.6
#include "SimpleRSLK.h"
#include <Servo.h>





//Clone variables, check if needed for Milestone 1

int LeftYValue, RightYValue;
int pos = 0;
float SpeedSettingR = 0;
float SpeedSettingL = 0;
int stopDistance = 5;
int object = 0;
float error2;
float motorSpeed;
float distIN;
const uint8_t lineColor = LIGHT_LINE;
const uint16_t normalSpeed = 10;
const uint16_t fastSpeed = 20;
uint32_t linePos = getLinePosition();
bool isCalibrationComplete = false;
//End

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
* Start STATE MACHINE VARIABLES
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

// List of given defined variables
#define UART_SERIAL Serial

const uint16_t normalSpeed = 10;
const uint16_t fastSpeed = 20;
const uint32_t targetTicks = 5000; // approx. 10'

//State machine variables 
#define IDLE 0
#define MANUAL 1
#define AUTO 2

int STATE = IDLE;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
* END MOTOR VARIABLES
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
* Start PLAYSTATION VARIABLES
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

//Define pin numbers for the button on the Playstation controller
#define PS2_DAT 14  //P1.7 <-> brown wire
#define PS2_CMD 15  //P1.6 <-> orange wire
#define PS2_SEL 34  //P2.3 <-> yellow wire (also called attention)
#define PS2_CLK 35  //P6.7 <-> blue wire
#define PS2X_DEBUG
#define PS2X_COM_DEBUG

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons 
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
//#define pressures true
#define pressures false
//#define rumble true
#define rumble false

//Create an instance of the Playstation controller  and Servo object
PS2X ps2x;  
Servo myservo;

int error = 0;
byte type = 0;
byte vibrate = 0;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
* END PLAYSTATION VARIABLES
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */ 

// Define remote mode either playstation controller or IR remote controller
enum RemoteMode 
{
  PLAYSTATION,
  IR_REMOTE,
};

// Declare and initialize the current state variable
RemoteMode CurrentRemoteMode = PLAYSTATION;

void setup() 
{
  // Set Serial communication to 115200 baud rate for MSP432
  UART_SERIAL.begin(115200);
  delay(500);
  UART_SERIAL.println("Initializing.....");

  //Run setup code
  setupRSLK();

  //Initialize LED pins as outputs
  setupLed(GREEN_LED);

  //Initialize LaunchPad buttons as inputs
  pinMode(LP_LEFT_BTN, INPUT_PULLUP);

  //Finialize initializiation
  UART_SERIAL.println("Initializing System Complete.");

  Serial.begin(57600);  //ZJE: changed from Arduino deafult of 9600
  Serial.print("Starting up Robot code.......");

  if (CurrentRemoteMode == 0)
  {
    Serial.println("Using Playstation Controller, make sure it is paired first ");
    setupPlaystationController(); //Start up the Playstation Controller
  }
  else if (CurrentRemoteMode == 1)
  {
    // TO BE IMPLEMENTED
  }
  
}




void loop() 
{
  //Read input from Playstation Controller
  ps2x.read_gamepad();

  //Operate the robot in remote control mode
  if (CurrentRemoteMode == 0)
  {
    Serial.println("Running remote control with the Playstation Controller");
    RemoteControlPlaystation();
  }
  else if (CurrentRemoteMode == 1)
  {
    // TO BE IMPLEMENTED
  }
}

/* RemoteControlPlaystation() function
  This function uses a playstation controller and the PLSK libraray with
  an RLSK robot using to implement remote controller. 
  
  A few actions are programed for an example. 

  Button control map:
  PAD UP button moves both motors forward
  CROSS button stops motors
*/
  void RemoteControlPlaystation() 
  {
    // put your code here to run in remote control mode

    // Example of receive and decode remote control command
    // the forward() and stop() functions should be independent of
    // the control methods
    if (ps2x.Button(PSB_PAD_UP)) 
    {
      Serial.println("PAD UP button pushed ");
      forward();
    } else if (ps2x.Button(PSB_CROSS)) 
    {
      Serial.println("CROSS button pushed");
      stop();
    }
  }

void setupPlaystationController()
{
  delayMicroseconds(500 * 1000);  //added delay to give wireless ps2 module some time to startup, before configuring it
  
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = 1;
  while (error) 
  {
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);

    switch (error)
    {
      case 0:
        Serial.print("Found Controller, configured successful ");
        Serial.print("pressures = ");
        if (pressures) ? Serial.println("true ") : Serial.println("false ");
        Serial.print("rumble = ");
        if (rumble) ? Serial.println("true ") : Serial.println("false ");
        break;
      case 1:
        Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
        break;
      case 2:
        Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
        break;
      case 3:
        Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
        break;
      default:
        break;
    }
    delayMicroseconds(1000 * 1000);
  }
    
  setupWaitBtn(LP_LEFT_BTN);
  setupLed(RED_LED);

  //  Serial.print(ps2x.Analog(1), HEX);

  type = ps2x.readType();
  switch (type) 
  {
    case 0:
      Serial.print("Unknown Controller type found ");
      break;
    case 1:
      Serial.print("DualShock Controller found ");
      break;
    case 2:
      Serial.print("GuitarHero Controller found ");
      break;
    case 3:
      Serial.print("Wireless Sony DualShock Controller found ");
      break;
    default:
      break;
  }
}