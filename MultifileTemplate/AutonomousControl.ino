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
  while (currentAutoState != 2)
  {
    switch (currentAutoState)
    {
      case 0:
        currentAutoState = 1;
        previousEvent = millis();
        break;

      case 1:
        lineFollowMode();
        
        if (ps2x.Button(PSB_SELECT))
        {
          currentState = 0;
          currentAutoState = 2;
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

  if (position > 0 && position < 4000)
  {
    setMotorSpeed(LEFT_MOTOR, normalSpeed);
    setMotorSpeed(RIGHT_MOTOR, fastSpeed);
  }
  else if (position > 5000)
  {
   setMotorSpeed(LEFT_MOTOR, fastSpeed);
   setMotorSpeed(RIGHT_MOTOR, normalSpeed);
  }
  else
  {
    setMotorSpeed(BOTH_MOTORS, normalSpeed);
  }
}


