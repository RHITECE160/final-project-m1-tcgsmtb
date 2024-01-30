/*
  AutonomousControl.ino - File that controls the robot autonomously
  
  Description: 
  This sketch is the central frame of the robot's behavior
  and relies on the other sketches. It sets up and runs the 
  movement, state machine, and IR interactions.

  Created by: Matt Bonilla, Mitch Mabardy, and Evan O'Brien 
  Date: 1/18/2024
  Version: 1.0
*/

//Load used libraries 
#include "PS2X_lib.h"  
#include "SimpleRSLK.h"
#include <Servo.h>
#include <TinyIRremote.h>

//Define the IR pins
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define IR_RCV_PIN 5 //P4.1
#define IR_TRX_PIN 18 //P5.1

//Create IR data structures
IRreceiver irRX(IR_RCV_PIN);
IRsender sendIR(IR_TRX_PIN);
IRData IRresults;
IRData IRmsg;
uint16_t IRaddress;           ///< Decoded address
uint16_t IRcommand;           ///< Decoded command

//Create and set the photoresistor 
const int phoRes = 2; //6.0
//const int resLED = 18; //3.0
int resVal = analogRead(phoRes);

//Create and set IR distance sensor
#define distSens 26 //4.4

//Define the Playstation controller pins 
#define PS2_DAT 14  //P1.7 <-> brown wire
#define PS2_CMD 15  //P1.6 <-> orange wire
#define PS2_SEL 34  //P2.3 <-> yellow wire (also called attention)
#define PS2_CLK 35  //P6.7 <-> blue wire

//Create Playstation data structure
PS2X ps2x;  // create PS2 Controller Class
// bool pressures = false;
// bool rumble = false;
int error = 1;

//Create Servo data structure
Servo myservo; // create Servo Class

//Create the options for Manual-Autonomous states
enum stateSel
{
  MANUAL,
  AUTO
};

enum autoStateSel
{
  GO,
  LINEFOLLOW,
  IDLE
};

//Create state machine variables 
stateSel currentState = MANUAL;
autoStateSel currentAutoState = GO;

//Create calibration variable
bool isCalibrationComplete = false;

//Create speed variables
const uint16_t stopped = 0;
const uint16_t normalSpeed = 10;
const uint16_t fastSpeed = 20;

//Create line-following variables
const uint8_t lineColor = LIGHT_LINE;
uint32_t linePos = getLinePosition();

//Create remaining possibily-needed variables
int stopDistance = 5; //Determins how far from a wall the robot will stop

void setup() 
{
  //Initialize the serial monitor
  Serial.begin(57600);
  Serial.println("Initializing Serial Monitor!");

  //Initialize the RSLK code
  setupRSLK();

  //Check if IR is ready to transmit signals
  if (sendIR.initIRSender())
  {
    Serial.println(F("Ready to Transmit NEC IR signals on pin " STR(IR_TRX_PIN)));
  }
  else 
  {
    Serial.println("Initialization of IR Transmitter Failed.");
    while (1) {;}
  }

  delay(500);
  enableTXLEDFeedback(GREEN_LED);
  IRmsg.protocol = NEC;
  IRmsg.command = IRcommand;
  IRmsg.address = IRaddress;
  IRmsg.isRepeat = false;

  //Check if IR is ready to receive signals
  if (irRX.initIRReceiver())
  {
    Serial.println(F("Ready to Receiver NEC IR signals on pin " STR(IR_RCV_PIN)));
  }
  else
  {
    Serial.println("Initialization of IR receiver Failed.");
    while (1) {;}
  }
  Serial.println("IR transmittion and receiver completed");

  delay(500);
  //enableRXLEDFeedback(BLUE_LED);

  //Initialize the photoresistor
  //pinMode(resLED, OUTPUT);
  pinMode(phoRes, INPUT);
  Serial.println("Photoresistor Initialized");

  //Initialize the servo
  myservo.attach(38);
  Serial.println("Servo Initialized");


  while (error) 
  {
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT);
    if (error == 0) 
      Serial.print("Found Controller, configured successful ");
    else if (error == 1)
      Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
    else if (error == 2)
      Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
    else if (error == 3) 
      Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
    delayMicroseconds(1000 * 1000);
  }
}

void loop() 
{
  //Read Playstation controller input
  ps2x.read_gamepad();

  //Perform respective state-machine state
  performStateMachine();
  // if (ps2x.Button(PSB_L1))
  //   votiveCandle();
  //if (ps2x.Button(PSB_R1))
    //catrinaCandle();

}

//Switches and performs the actions of the state machine
void performStateMachine()
{
  switch (currentState)
  {
    case MANUAL:

      //Serial.println("Entering Manual Mode");
      playStationControls();
      if (ps2x.Button(PSB_SELECT))
        currentState = AUTO;
      break;

    case AUTO:

      Serial.println("Entering Autonomous Mode");
      autoControls();
      if (ps2x.Button(PSB_SELECT))
        currentState = MANUAL;
      break;  

    default:
      break;
  }
}

//Interprets future robot move via button presses on the Playstation controller
void playStationControls()
{
  if (ps2x.Button(PSB_PAD_UP)) //Makes the robot move forward
  {
    Serial.println("Moving Forward");
    forward();
  } 
  else if (ps2x.Button(PSB_PAD_DOWN)) //Makes the robot move backward
  {
    Serial.println("Moving Backward");
    backward();
  }
  else if (ps2x.Button(PSB_PAD_RIGHT)) //Makes the robot turn right
  {
    Serial.println("Turning Right");
    turnRight();
  }
  else if (ps2x.Button(PSB_PAD_LEFT)) //Makes the robot turn left
  {
    Serial.println("Turning Left");
    turnLeft();
  }
  else if (ps2x.Button(PSB_CROSS)) //Makes the robot stop
  {
    Serial.println("Stopping Robot");
    stop();
  }
  else if (ps2x.Button(PSB_CIRCLE)) //Makes the robot spin
  {
    Serial.println("Spinning Robot");
    spin();
  }
  else if (ps2x.Button(PSB_R2))
  {
    Serial.println("Opening Gripper");
    gripperOpen();
  }
  else if (ps2x.Button(PSB_L2))
  {
    Serial.println("Closing Gripper");
    gripperClose();
  }
}