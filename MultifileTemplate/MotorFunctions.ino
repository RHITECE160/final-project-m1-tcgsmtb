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
void forward() {
  enableMotor(BOTH_MOTORS);
  setMotorDirection(LEFT_MOTOR, MOTOR_DIR_FORWARD);
  setMotorDirection(RIGHT_MOTOR, MOTOR_DIR_FORWARD);
  setMotorSpeed(BOTH_MOTORS, fastSpeed);
}

//Move the robot backward
void backward() {
  enableMotor(BOTH_MOTORS);
  setMotorDirection(LEFT_MOTOR, MOTOR_DIR_BACKWARD);
  setMotorDirection(RIGHT_MOTOR, MOTOR_DIR_BACKWARD);
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
  for (int i = 40; i < 140; i++)
    myservo.write(i);
}

//Closes the servo gripper
void gripperClose()
{
  for (int i = 140; i > 40; i--)
    myservo.write(i);
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
    IRcommand = 0x61;
  else IRcommand 0xA0 //Regular Votive Candle

  //Send IR data
  IRmsg.address = 0xEE;
  IRmsg.command = IRcommand;
  sendIR.write(&IRmsg);
  delay(500);
}
