/*
  AutonomousControl.ino - File that controls the robot autonomously
  
  Description:
  This sketch deals with the autonomous mode of the robot
  and the line following capabilities it has.

  Created by: Matt Bonilla, Mitch Mabardy, and Evan O'Brien
  Date: 1/18/2024
  Version: 1.0
*/


//Logs the time of the previous autonomous event
unsigned long previousEvent;

//Used for the storage of line following data
uint32_t position; 
int direction;

void autoControls()
{
  while (currentAutoState != IDLE)
  {
    switch (currentAutoState)
    {
      case GO:
        currentAutoState = LINEFOLLOW;
        previousEvent = millis();
        break;

      case LINEFOLLOW:
        lineFollowMode();
        
        if (ps2x.Button(PSB_START))
        {
          currentState = MANUAL;
          currentAutoState = GO;
        }
        break;

      default:
        break;
    }
  }
}

void lineFollowMode()
{
  position = getLinePosition();
  direction = 0;

  if (position > 3600 && position < 4475)
  {
    enableMotor(BOTH_MOTORS);
    setMotorDirection(BOTH_MOTORS, MOTOR_DIR_FORWARD);
    setMotorSpeed(LEFT_MOTOR, 5);
    setMotorSpeed(RIGHT_MOTOR, 7);
  }
  else if (position > 4525 && position < 5400)
  {
    enableMotor(BOTH_MOTORS);
    setMotorDirection(BOTH_MOTORS, MOTOR_DIR_FORWARD);
    setMotorSpeed(LEFT_MOTOR, 7);
    setMotorSpeed(LEFT_MOTOR, 5);
  }
  else if (position < 3600)
  {
    setMotorSpeed(BOTH_MOTORS, 0);
    delay(500);
    enableMotor(LEFT_MOTOR);
    setMotorDirection(LEFT_MOTOR, MOTOR_DIR_BACKWARD);
    setMotorSpeed(BOTH_MOTORS, 8);
  }
  else if (position > 5400)
  {
    setMotorSpeed(BOTH_MOTORS, 0);
    delay(500);
    enableMotor(RIGHT_MOTOR);
    setMotorDirection(RIGHT_MOTOR, MOTOR_DIR_BACKWARD);
    setMotorSpeed(BOTH_MOTORS, 8);
  }
  else
    setMotorSpeed(BOTH_MOTORS, 7);
}

