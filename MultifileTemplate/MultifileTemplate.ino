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


/*
Pin Map:

P1.7 - Playstation Brown Wire
P1.6 - Playstation Orange Wire
P2.3 - Playstation Yellow Wire
P6.7 - Playstation Blue Wire

P4.1 - IR Receiver 
P3.0 - IR Transmitter

P4.4 - Ultrasonic 

P5.1 - Photoresistor
*/

//Load used libraries
#include "PS2X_lib.h"
#include "SimpleRSLK.h"
#include <Servo.h>
#include <TinyIRremote.h>
#include <Ultrasonic.h>

//Define the IR pins
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define IR_RCV_PIN 5   //P4.1
#define IR_TRX_PIN 18  //P3.0

//Create IR data structures
IRreceiver irRX(IR_RCV_PIN);
IRsender sendIR(IR_TRX_PIN);
IRData IRresults;
IRData IRmsg;
uint16_t IRaddress;  ///< Decoded address
uint16_t IRcommand;  ///< Decoded command



//Create and set Ultrasonic distance sensor
#define distSens 26  //4.4

//Setup ultrasonic 
Ultrasonic ultrasonic(26);
int distIn;
int motorSpeed;

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
Servo myservo;  // create Servo Class

//Create the options for Manual-Autonomous states

int currentState = 0;
/* currentState
* 0 - Manual
* 1 - Auto
*/

int currentAutoState = 0;
/* currentAutoState
* 0 - Go
* 1 - LineFollow
* 2 - Idle
*/

//Create remaining possibily-needed variables
int stopDistance = 5;  //Determins how far from a wall the robot will stop

//Logs the time of the previous autonomous event
unsigned long previousEvent;

//Create calibration variable
bool isCalibrationComplete = false;

//Create speed variables
const uint16_t stopped = 0;
const uint16_t normalSpeed = 10;
const uint16_t fastSpeed = 20;

//Used for the storage of line following data
uint32_t linePos; 
int direction;
const uint8_t lineColor = LIGHT_LINE;

/*
1). Upload file(s)
2). Turn chasis on
3). Turn controller on
4). Hit reset button - implement a wait button to prevent setup() from running before steps 1-3
5). Hit the start button the Playstation controller
*/

void setup() {
  //Initialize the serial monitor
  Serial.begin(57600);
  Serial.println("Initializing Serial Monitor!");

  //Initialize the RSLK code
  setupRSLK();

  setupLed(RED_LED);

  //Check if IR is ready to transmit signals
  if (sendIR.initIRSender()) {
    Serial.println(F("Ready to Transmit NEC IR signals on pin " STR(IR_TRX_PIN)));
  } else {
    Serial.println("Initialization of IR Transmitter Failed.");
    while (1) { ; }
  }

  delay(500);
  enableTXLEDFeedback(GREEN_LED);
  IRmsg.protocol = NEC;
  IRmsg.command = IRcommand;
  IRmsg.address = IRaddress;
  IRmsg.isRepeat = false;

  //Check if IR is ready to receive signals
  if (irRX.initIRReceiver()) {
    Serial.println(F("Ready to Receiver NEC IR signals on pin " STR(IR_RCV_PIN)));
  } else {
    Serial.println("Initialization of IR receiver Failed.");
    while (1) { ; }
  }
  Serial.println("IR transmittion and receiver completed");

  delay(500);
  //enableRXLEDFeedback(BLUE_LED);

  //Initialize the servo
  myservo.attach(38);
  Serial.println("Servo Initialized");


  while (error) {
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

void loop() {
  //Read Playstation controller input
  ps2x.read_gamepad();

  // distIn = ultrasonic.read();
  // if (distIn < 10)
  // {
  //   stop();
  // }


  // if (isCalibrationComplete == false) {
  //   floorCalibration();
  //   isCalibrationComplete == true;
  // }

  //Perform respective state-machine state
  performStateMachine();
  if (ps2x.Button(PSB_L1))
    votiveCandle();
  if (ps2x.Button(PSB_R1))
    catrinaCandle();
}

//Switches and performs the actions of the state machine
void performStateMachine() {
  switch (currentState) {
    case 0:
      Serial.println("Entering Manual Mode");
      playStationControls();
      if (ps2x.Button(PSB_SELECT)) {
        Serial.println(" from current state = 0 ->Select button pushed");
        currentState = 1;
      }
      break;

    case 1:
      Serial.println("Entering Autonomous Mode");
      autoControls();
      if (ps2x.Button(PSB_SELECT))
        currentState = 0;
      break;

    default:
      currentState = 1;
      break;
  }
}

//Interprets future robot move via button presses on the Playstation controller
void playStationControls() {
  if (ps2x.Button(PSB_PAD_UP))  //Makes the robot move forward
  {
    Serial.println("Moving Forward");
    forward();
  } else if (ps2x.Button(PSB_PAD_DOWN))  //Makes the robot move backward
  {
    Serial.println("Moving Backward");
    backward();
  } else if (ps2x.Button(PSB_PAD_RIGHT))  //Makes the robot turn right
  {
    Serial.println("Turning Right");
    turnRight();
  } else if (ps2x.Button(PSB_PAD_LEFT))  //Makes the robot turn left
  {
    Serial.println("Turning Left");
    turnLeft();
  } else if (ps2x.Button(PSB_CROSS))  //Makes the robot stop
  {
    Serial.println("Stopping Robot");
    stop();
  } else if (ps2x.Button(PSB_CIRCLE))  //Makes the robot spin
  {
    Serial.println("Spinning Robot");
    spin();
  } else if (ps2x.Button(PSB_R2)) {
    Serial.println("Opening Gripper");
    gripperOpen();
  } else if (ps2x.Button(PSB_L2)) {
    Serial.println("Closing Gripper");
    gripperClose();
  }
}