/*
  AutonomousControl.ino - File that controls the robot autonomously
  
  Description: 
  This sketch is the central frame of the robot's behavior
  and relies on the other sketches. It sets up and runs the 
  movement, state machine, and IR interactions.

  Created by: Matt Bonilla, Mitch Mabardy, and Evan O'Brien 
  Date: 1/18/2024
  Last Revised: 2/05/24
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
#define IR_TRX_PIN 18 //P6.0
#define ledPin 2

//Create IR data structures
IRreceiver irRX(IR_RCV_PIN);
IRsender sendIR(IR_TRX_PIN);
IRData IRresults;
IRData IRmsg;
uint16_t IRaddress;  ///< Decoded address
uint16_t IRcommand;  ///< Decoded command
volatile bool justWritten = false;



//Create and set Ultrasonic distance sensor
#define distSens 26  //4.4
Ultrasonic ultrasonic(distSens);

//Setup ultrasonic 
int distIn;
int turnInTunnelTime;
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
bool isInTunnel = false;

int tunnelState = 0;
//idk if this is the best way to do this 
/* tunnelState
* 0 - entering tunnel from starting area
* 1 - entering tunnel from cemetery area
*/

int currentState = 0;
/* currentState
* 0 - Manual
* 1 - Auto
*/

int currentAutoState = 0;
/* currentAutoState
* 0 - LineFollow
* 1 - Tunnel
* 2 - Idle
*/

//Create remaining possibily-needed variables
int stopDistance = 10;  //Determins how far from a wall the robot will stop

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

//Photoresistor
#define photoRes 33 //P5.1
int phoValue;

/*
1). Upload file(s)
2). Turn chasis on
3). Turn controller on
4). Hit reset button - implement a wait button to prevent setup() from running before steps 1-3
5). Hit the start button the Playstation controller
*/

void setup() {

  // set pin mode for IR LED transmitter
  pinMode(ledPin, OUTPUT);

  //Initialize the serial monitor
  Serial.begin(57600);
  Serial.println("Initializing Serial Monitor!");

  //Initialize the RSLK code
  setupRSLK();



  //Check if IR is ready to transmit signals
  if (sendIR.initIRSender()) {
    Serial.println(F("Ready to Transmit NEC IR signals on pin " STR(IR_TRX_PIN)));
  } else {
    Serial.println("Initialization of IR Transmitter Failed.");
    while (1) { ; }
  }

  //Check if IR is ready to receive signals
  if (irRX.initIRReceiver(true, true, handleReceivedTinyIRData)) {
    Serial.println(F("Ready to Receiver NEC IR signals on pin " STR(IR_RCV_PIN)));
  } else {
    Serial.println("Initialization of IR receiver Failed.");
    while (1) { ; }
  }
  Serial.println("IR transmittion and receiver completed");

  delay(500);
  IRmsg.protocol = NEC;
  IRmsg.command = IRcommand;
  IRmsg.address = IRaddress;
  IRmsg.isRepeat = false;

  delay(500);
  //enableRXLEDFeedback(BLUE_LED);

  //Initialize the servo
  myservo.attach(38);
  Serial.println("Servo Initialized");

  //Photoresistor
  phoValue = analogRead(photoRes);
  Serial.println("Photoresistor Initialized");

  while (error) {
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT);
    if (error == 0)
      Serial.println("Found Controller, configured successful ");
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
  phoValue = analogRead(photoRes);
  //Serial.println(phoValue);

  linePos = getLinePosition();
  //Serial.println((linePos - 4000));

  // if (phoValue < 150)
  // {
  //   currentState = 1;
  //   currentAutoState = 1;
  // }
  // distIn = ultrasonic.read();
  // if (distIn < 10)
  // {
  //   stop();
  // }

  //Calibrate 
  // if (isCalibrationComplete == false) {
  //   floorCalibration();
  //   isCalibrationComplete == true;
  // }

  //Perform respective state-machine state
  improvedStateMachine();
  
}


void improvedStateMachine()
{
  switch (currentState)
  {
    case 0: //Manual Mode
      Serial.println("Entering Manual Mode");

      playStationControls();

      if (ps2x.Button(PSB_SQUARE))
      {
        Serial.println("Pressed Square Button in Manual Mode, changing to Tunnel Drive Mode");
        currentState = 1;
        currentAutoState = 2;
      }
      break;

    case 1: //Autonomous Mode
      Serial.print("Entering Tunnel Drive Mode: ");
      // currentAutoState = 0; //Change autonomous state to Line Following 
      Serial.println(ultrasonic.read());
      tunnelMode();

      // while (!(ps2x.Button(PSB_START))) //Has autonomous controls until start is pressed
      // {
      //   improvedAutoControls();
      // } //End while-loop

      // Serial.println("Pressed Select Button in Autonomous Mode"); //If start pressed, will exit the while-loop
      // currentState = 0; //Change standard state to Manual Mode
      break;

    default:
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
  } else if (ps2x.Button(PSB_R2)) //Opens the robot's gripper
  {
    Serial.println("Opening Gripper");
    gripperOpen();
  } else if (ps2x.Button(PSB_L2)) //Closes the robot's gripper
  {
    Serial.println("Closing Gripper");
    gripperClose();
  } else if (ps2x.Button(PSB_L1)) //Activates the Gold Votive Candle
  {
    Serial.println("Lighting Gold Votive");
    goldVotiveCandle();
  } else if (ps2x.Button(PSB_R1)) //Activates the Catrina Candle
  {
    Serial.println("Lighting Catrina Candle");
    catrinaCandle();
  } else if (ps2x.Button(PSB_R3)) //Turns on the Black Votive Candle
  {
    Serial.println("Turning on Black Votive");
    blackVotiveCandleOn();
  } else if (ps2x.Button(PSB_L3)) //Turn off the Black Votive Candle
  {
    Serial.println("Turning off Black Votive");
    blackVotiveCandleOff();
  }
  delay(10);
}

void handleReceivedTinyIRData(uint16_t aAddress, uint8_t aCommand, bool isRepeat) {
    /*
     * Since we are in an interrupt context and do not want to miss the next interrupts 
     *  of the repeats coming soon, quickly save data and return to main loop
     */
    IRresults.address = aAddress;
    IRresults.command = aCommand;
    IRresults.isRepeat = isRepeat;
    // Let main function know that new data is available
    justWritten = true;
}