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
        lineFollowMode();
        
        if (ps2x.Button(PSB_SELECT))
        {
          currentState = 0;
          currentAutoState = 2;
        }
        break;

      case 1:
        tunnelMode();
        currentAutoState = 2;
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

  if (linePos > 0 && linePos < 4000)
  {
    setMotorSpeed(LEFT_MOTOR, normalSpeed);
    setMotorSpeed(RIGHT_MOTOR, fastSpeed);
  }
  else if (linePos > 5000)
  {
   setMotorSpeed(LEFT_MOTOR, fastSpeed);
   setMotorSpeed(RIGHT_MOTOR, normalSpeed);
  }
  else
  {
    setMotorSpeed(BOTH_MOTORS, normalSpeed);
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

void distanceSensorMode() {
  if (isInTunnel){
    distIn = ultrasonic.read();

    if (distIn > stopDistance) forward();

    else if (distIn <= stopDistance && distIn != 0) {
      turnLeft();
      delay(turnInTunnelTime);
      forward();
    }

  }else {
    

  }

}

void tunnelMode()
{
  while(isInTunnel)
  {
    distIn = ultrasonic.read();
    switch (tunnelState)
    {
      case 0:
        while (distIn > stopDistance) {
        forward();
        }
        turnRight();
        while (phoValue < 0) {
          forward();
        }
        stop();
        tunnelState = 1;
        break;
      case 1:
      {
        while (distIn > stopDistance) {
          forward();
        }
        turnLeft();
        while (phoValue < 0) {
          forward();
        } 
        stop();
        tunnelState = 0;
      }
      default:
        break;
    }
  }
}

void dropOffBlock() {

}
