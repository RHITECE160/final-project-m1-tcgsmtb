/*
  AutonomousControl.ino - File that controls the robot autonomously
  
  Description:
  This sketch deals with the autonomous mode of the robot
  and the line following capabilities it has.

  Created by: Matt Bonilla, Mitch Mabardy, and Evan O'Brien
  Date: 1/18/2024
  Version: 1.0
*/




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
  linePos = getLinePosition();
  direction = 0;

  if ((linePos > 0) && (linePos < 4000)) {    // turn left
        setMotorSpeed(LEFT_MOTOR, normalSpeed);
        setMotorSpeed(RIGHT_MOTOR, fastSpeed);
    } else if (linePos > 5000) {                // turn right
        setMotorSpeed(LEFT_MOTOR, fastSpeed);
        setMotorSpeed(RIGHT_MOTOR, normalSpeed);
    } else {                                    // go straight
        setMotorSpeed(LEFT_MOTOR, normalSpeed);
        setMotorSpeed(RIGHT_MOTOR, normalSpeed);
    }
}

void floorCalibration()
{
    /* Place Robot On Floor (no line) */
    delay(2000);
    Serial.println("Push left button on Launchpad to begin calibration.");
    Serial.println("Make sure the robot is on the floor away from the line.\n");

    delay(500);
    Serial.println("Running calibration on floor");

    /* Set both motors direction forward */
    setMotorDirection(BOTH_MOTORS, MOTOR_DIR_FORWARD);
    /* Enable both motors */
    enableMotor(BOTH_MOTORS);
    /* Set both motors speed 20 */
    setMotorSpeed(BOTH_MOTORS, 20);

    /* Must be called prior to using getLinePosition() or readCalLineSensor() */
    calibrateLineSensor(lineColor);

    /* Disable both motors */
    disableMotor(BOTH_MOTORS);

    Serial.println("Reading floor values complete");

    Serial.println("Push left button on Launchpad to begin line following.");
    Serial.println("Make sure the robot is on the line.\n");
    /* Wait until button is pressed to start robot */
    waitBtnPressed(LP_LEFT_BTN, RED_LED);
    delay(1000);

    enableMotor(BOTH_MOTORS);
}
