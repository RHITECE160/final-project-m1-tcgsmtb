/*
  AutonomousControl.ino - File that controls the robot autonomously
  
  Description:
  This sketch deals with the autonomous mode of the robot
  and the line following capabilities it has.

  Created by: Matt Bonilla, Mitch Mabardy, and Evan O'Brien
  Date: 1/18/2024
  Last Revised: 2/08/24
  Version: 1.0
*/




// void autoControls()
// {
//   while (currentAutoState != 2)
//   { 
//     switch (currentAutoState)
//     {
//       case 0:
//         Serial.println("Entering Line Following Mode");
//         lineFollowMode();
        
        
//         if (ps2x.Button(PSB_SELECT))
//         {
//           currentState = 0;
//           currentAutoState = 2;
//         }
//         break;

//       case 1:
//         tunnelMode();
//         currentState = 0;
//         currentAutoState = 2;
//         break;

//       default:
//         break;
//     }
//   }
// }

/* currentAutoState
* 0 - LineFollow
* 1 - Tunnel
* 2 - Idle
*/
void improvedAutoControls()
{
  while (currentAutoState != 2)
  {
    switch (currentAutoState)
    {
      case 0: //Line Following Mode
        Serial.println("Entering Line Following Mode");

        while (!(ps2x.Button(PSB_START))) //Line follows until the start button is pressed
        {
          lineFollowMode();
        } //End while-loop
        break;

      case 1: //Tunnel Drive Mode
        Serial.println("Entering Tunnel Drive Mode");

        phoValue = analogRead(photoRes); //Reads an initial value from the photo resistor
        while (distIn > stopDistance)
        {
          distIn = ultrasonic.read();
          forward();
        } //End while-loop
        
        /* tunnelState
        * 0 - entering tunnel from starting area
        * 1 - entering tunnel from cemetery area
        */
        switch (tunnelState)
        {
          case 0: //Turns right if entering tunnel from starting area
            turnRight();
            tunnelState = 1; 
            break;
          
          case 1: //Turns left if entering the tunnel from cemetery area.
            turnLeft();
            tunnelState = 0;
            break;

          default:
            break;
        }

        while (phoValue < 500) //Continues forward until reaching a lighter area
        {
          phoValue = analogRead(photoRes);
          forward();
        } //End while-loop

        stop();
        currentState = 0; //Go to Manual Mode
        currentAutoState = 2; //Set Autonomous Mode to idle 
        break;

      default:
        break;
    }
  }
}

void lineFollowMode()
{
  linePos = getLinePosition();

  if (linePos - 4000 > 300 && linePos - 4000 < 1000)
  {
    forward();
  }
  else if (linePos - 4000 > 1000)
  {
   turnLeft();
  }
  else if (linePos - 4000 < 300)
  {
    turnRight();
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

void tunnelMode()
{
  //Assumption: Starts method when on starting area side
  distIn = ultrasonic.read();

  while (distIn > stopDistance)  //Moves forward until hitting the corner of the tunnel
  {
    distIn = ultrasonic.read();
    Serial.println(distIn);
    backward();
  }
  Serial.println("Turning Right");

  distIn = ultrasonic.read();

  stop();
  turnRight(); //Turns right, supposedly is facing the cemetery now.
  delay(1000);

  while (distIn > 40) //Moves forward until reaching the cemetery
  {
    distIn = ultrasonic.read();
    Serial.println(distIn);
    backward();
  }
  Serial.println("Exiting Tunnel");

  distIn = ultrasonic.read();
  spin(); //will spin and leave marigold in the cemetery
  delay(3000);


  while (distIn > stopDistance) //Moves forward until hitting the corner of the tunnel
  {
    distIn = ultrasonic.read();
    Serial.println(distIn);
    backward();
  }
  Serial.println("Turning Left");

  distIn = ultrasonic.read();

  stop();
  turnLeft(); //Turns left, supposedly is facing the cemetery now.
  delay(1000);

  while (distIn < 40) //Moves forward until reaching the exit of the tunnel
  {
    distIn - ultrasonic.read();
    Serial.println(distIn);
    backward();
  }
  Serial.println("Exiting Tunnel");
  currentState = 0;
}
