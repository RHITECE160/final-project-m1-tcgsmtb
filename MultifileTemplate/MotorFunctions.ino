  /*
  MotorFunctions.ino - Arduino Sketch for Motor Control
  
  Description:
  This sketch provides functions for controlling a RLSK robot, allowing the
  motors to move forward and stop.

  Functions:
  1. void moveForward()
     - Activates the motor to move forward.

  2. void stopMotor()
     - Stops the motor.

  Created by: Matt Bonilla, Mitch Mabardy, andEvan O'Brien
  Date: 1/18/2024
  Version: 1.0
*/

/* Moves robot forward: both motors forward same speed */

int gripperValue = 90;

void forward() {
  enableMotor(BOTH_MOTORS);
  setMotorDirection(BOTH_MOTORS, MOTOR_DIR_BACKWARD);
  // setMotorDirection(LEFT_MOTOR, MOTOR_DIR_BACKWARD);
  // setMotorDirection(RIGHT_MOTOR, MOTOR_DIR_BACKWARD);
  setMotorSpeed(BOTH_MOTORS, fastSpeed);
}

//Move the robot backward
void backward() {
  enableMotor(BOTH_MOTORS);
  setMotorDirection(BOTH_MOTORS, MOTOR_DIR_FORWARD);
  // setMotorDirection(RIGHT_MOTOR, MOTOR_DIR_FORWARD);
  setMotorSpeed(BOTH_MOTORS, fastSpeed);
}

//Turn the robot right
void turnRight() {
  enableMotor(LEFT_MOTOR);
  setMotorDirection(LEFT_MOTOR, MOTOR_DIR_FORWARD);
  setMotorSpeed(BOTH_MOTORS, fastSpeed);
}

//Turns the robot left
void turnLeft() {
  enableMotor(RIGHT_MOTOR);
  setMotorDirection(RIGHT_MOTOR, MOTOR_DIR_FORWARD);
  setMotorSpeed(BOTH_MOTORS, fastSpeed);
}

//Spins the robot 
void spin() {
  enableMotor(BOTH_MOTORS);
  setMotorDirection(LEFT_MOTOR, MOTOR_DIR_BACKWARD);
  setMotorDirection(RIGHT_MOTOR, MOTOR_DIR_FORWARD);
  setMotorSpeed(BOTH_MOTORS, fastSpeed);
}

/* Stops robot forward: both motors disabled */
void stop() {
  disableMotor(BOTH_MOTORS);
}

//Opens the servo gripper
void gripperOpen() 
{
  if (gripperValue < 180) gripperValue++;
    myservo.write(gripperValue);
}

//Closes the servo gripper
void gripperClose()
{
  if (gripperValue > 0) gripperValue--;
    myservo.write(gripperValue);
}

void catrinaCandle()
{
  //If command available, receive the command
  if (irRX.decodeIR(&IRresults))
    IRcommand = IRresults.command;

  //Send IR data
  IRmsg.address = 0xCE;
  IRmsg.command = IRcommand;
  sendIR.write(&IRmsg);
  delay(500);
}

void votiveCandle()
{
  //
  if (irRX.decodeIR(&IRresults) && IRresults.command == 160) //Gold Votive Candle
  {
    IRmsg.address = 0xEE;
    IRmsg.command = 0x61;
    sendIR.write(&IRmsg);
    delay(500);
  } 

  //Send IR data
  // IRmsg.address = 0xEE;
  // IRmsg.command = 0xA0;
  // sendIR.write(&IRmsg);
  // delay(500);
  Serial.println("sending IR signal");
  digitalWrite(2, HIGH);
  delay(1000);
  digitalWrite(2, LOW);
  Serial.println("turning off IR signal");
}
