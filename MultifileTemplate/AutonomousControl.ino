/*
  AutonomousControl.ino - File that controls the robot autonomously
  
  Description:
  This sketch provides functions for IR receiving and transmitting, as well as line following.

  Created by: Your Name
  Date: 1/18/2024
  Version: 1.0
*/

int LeftYValue, RightYValue;

void Left_MotorF() {
  setMotorDirection(LEFT_MOTOR, MOTOR_DIR_FORWARD);
  LeftYValue = ps2x.Analog(PSS_LY);
  LeftYValue = constrain(ps2x.Analog(PSS_LY), 0, 50);
  SpeedSettingL = map(LeftYValue, 115, 0, 0, 50);
  setMotorSpeed(LEFT_MOTOR, SpeedSettingL);
  ps2x.read_gamepad();
}

void Right_MotorF() {
  setMotorDirection(RIGHT_MOTOR, MOTOR_DIR_FORWARD);
  RightYValue = ps2x.Analog(PSS_RY);
  RightYValue = constrain(ps2x.Analog(PSS_RY), 0, 50);
  SpeedSettingR = map(RightYValue, 115, 0, 0, 50);
  setMotorSpeed(RIGHT_MOTOR, SpeedSettingR);
  ps2x.read_gamepad();
}

void Left_MotorB() {
  setMotorDirection(LEFT_MOTOR, MOTOR_DIR_BACKWARD);
  LeftYValue = ps2x.Analog(PSS_LY);
  LeftYValue = constrain(ps2x.Analog(PSS_LY), 0, -50);
  SpeedSettingL = map(LeftYValue, 135, 255, -0, -50);
  setMotorSpeed(LEFT_MOTOR, SpeedSettingL);
  ps2x.read_gamepad();
}

void Right_MotorB() {
  setMotorDirection(RIGHT_MOTOR, MOTOR_DIR_BACKWARD);
  RightYValue = ps2x.Analog(PSS_RY);
  RightYValue = constrain(ps2x.Analog(PSS_RY), 0, -50);
  SpeedSettingR = map(RightYValue, 135, 255, 0, -50);
  setMotorSpeed(RIGHT_MOTOR, SpeedSettingR);
  ps2x.read_gamepad();
}

void Both_MotorsF() {
  setMotorDirection(BOTH_MOTORS, MOTOR_DIR_FORWARD);
  RightYValue = ps2x.Analog(PSS_RY);
  RightYValue = constrain(ps2x.Analog(PSS_RY), 0, 50);
  SpeedSettingR = map(RightYValue, 115, 0, 0, 50);
  setMotorSpeed(BOTH_MOTORS, SpeedSettingR);
}

void Both_MotorsB() {
  setMotorDirection(BOTH_MOTORS, MOTOR_DIR_BACKWARD);
  RightYValue = ps2x.Analog(PSS_RY);
  RightYValue = constrain(ps2x.Analog(PSS_RY), 0, -50);
  SpeedSettingR = map(RightYValue, 135, 255, 0, -50);
  setMotorSpeed(BOTH_MOTORS, SpeedSettingR);
}

void handleReceivedTinyIRData(uint16_t aAddress, uint8_t aCommand, bool isRepeat) 
{
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

