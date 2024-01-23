#include "PS2X_lib.h"  //for v1.6
#include "SimpleRSLK.h"
#include <Servo.h>

#define UART_SERIAL Serial
#define PS2_DAT 14  //P1.7 <-> brown wire
#define PS2_CMD 15  //P1.6 <-> orange wire
#define PS2_SEL 34  //P2.3 <-> yellow wire (also called attention)
#define PS2_CLK 35  //P6.7 <-> blue wire
#define PS2X_DEBUG
#define PS2X_COM_DEBUG
#define IDLE 0
#define MANUAL 1
#define Auto_Right 2
#define Line_Follow 3

int STATE = IDLE;
int LeftYValue, RightYValue;
int pos = 0;
float SpeedSettingR = 0;
float SpeedSettingL = 0;
int stopDistance = 5;
int object = 0;
float error2;
float motorSpeed;
float distIN;
const uint8_t lineColor = LIGHT_LINE;
const uint16_t normalSpeed = 10;
const uint16_t fastSpeed = 20;
uint32_t linePos = getLinePosition();
bool isCalibrationComplete = false;


PS2X ps2x;  // create PS2 Controller Class
Servo myservo;
int error = 0;
byte type = 0;
byte vibrate = 0;

void setup() 
{
  UART_SERIAL.begin(115200);
  myservo.attach(SRV_0);
  Serial.begin(57600);  //ZJE: changed from Arduino deafult of 9600
  setupRSLK();
  delayMicroseconds(500 * 1000);  //added delay to give wireless ps2 module some time to startup, before configuring it
  error = 1;
  while (error) 
  {
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT);
    if (error == 0) 
    {
      Serial.print("Found Controller, configured successful ");
    } 
    else if (error == 1)
      Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");

    else if (error == 2)
      Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

    else if (error == 3)
      Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
    delayMicroseconds(1000 * 1000);
  }
  setupWaitBtn(LP_LEFT_BTN);
  setupLed(RED_LED);

  //  Serial.print(ps2x.Analog(1), HEX);

  type = ps2x.readType();
  switch (type) {
    case 0:
      Serial.print("Unknown Controller type found ");
      break;
    case 1:
      Serial.print("DualShock Controller found ");
      break;
    case 2:
      Serial.print("GuitarHero Controller found ");
      break;
    case 3:
      Serial.print("Wireless Sony DualShock Controller found ");
      break;
  }
}




void loop() 
{
  if (isCalibrationComplete == false) 
  {
    floorCalibration();
    isCalibrationComplete = true;
  }
  uint32_t linePos = getLinePosition();
  if (error == 1)  //skip loop if no controller found
    return;

  if (type != 2) 
  {
    ps2x.read_gamepad();
    switch (STATE) 
    {
      case IDLE:
        ps2x.read_gamepad();
        if (ps2x.Button(PSB_L1)) 
        {
          STATE = MANUAL;
        }
        break;
      case MANUAL:
        enableMotor(2);
        ps2x.read_gamepad();
        if (ps2x.Analog(PSS_LY) < 115 && ps2x.Analog(PSS_RY) < 115) 
        {
          Both_MotorsF();
        } 
        else if (ps2x.Analog(PSS_LY) > 135 && ps2x.Analog(PSS_RY) > 135) 
        {
          Both_MotorsB();
        } 
        else if (ps2x.Analog(PSS_LY) < 115) 
        {
          Left_MotorF();
        } 
        else if (ps2x.Analog(PSS_LY) > 135) 
        {
          Left_MotorB();
        } 
        else if (ps2x.Analog(PSS_RY) < 115) 
        {
          Right_MotorF();
        } 
        else if (ps2x.Analog(PSS_RY) > 135) 
        {
          Right_MotorB();
        } 
        else if (ps2x.NewButtonState()) 
        {
          Gripper();
          if (ps2x.ButtonPressed(PSB_CROSS)) 
          {
            STATE = Auto_Right;
          }
          if (ps2x.Button(PSB_R1)) 
          {
            STATE = Line_Follow;
          }
        } 
        else 
        {
          disableMotor(BOTH_MOTORS);
        }
        break;
      case Auto_Right:
        Auto_Turn_Right();
        if (ps2x.Button(PSB_L1)) 
        {
          object = 0;
          STATE = MANUAL;
        }
        break;
      case Line_Follow:
        Line_Following();
        break;
    }
    delayMicroseconds(50 * 1000);
  }
}

  void Left_MotorF() 
  {
    setMotorDirection(LEFT_MOTOR, MOTOR_DIR_FORWARD);
    LeftYValue = ps2x.Analog(PSS_LY);
    LeftYValue = constrain(ps2x.Analog(PSS_LY), 0, 50);
    SpeedSettingL = map(LeftYValue, 115, 0, 0, 50);
    setMotorSpeed(LEFT_MOTOR, SpeedSettingL);
    ps2x.read_gamepad();
  }

  void Right_MotorF() 
  {
    setMotorDirection(RIGHT_MOTOR, MOTOR_DIR_FORWARD);
    RightYValue = ps2x.Analog(PSS_RY);
    RightYValue = constrain(ps2x.Analog(PSS_RY), 0, 50);
    SpeedSettingR = map(RightYValue, 115, 0, 0, 50);
    setMotorSpeed(RIGHT_MOTOR, SpeedSettingR);
    ps2x.read_gamepad();
  }

  void Left_MotorB() 
  {
    setMotorDirection(LEFT_MOTOR, MOTOR_DIR_BACKWARD);
    LeftYValue = ps2x.Analog(PSS_LY);
    LeftYValue = constrain(ps2x.Analog(PSS_LY), 0, -50);
    SpeedSettingL = map(LeftYValue, 135, 255, -0, -50);
    setMotorSpeed(LEFT_MOTOR, SpeedSettingL);
    ps2x.read_gamepad();
  }

  void Right_MotorB() 
  {
    setMotorDirection(RIGHT_MOTOR, MOTOR_DIR_BACKWARD);
    RightYValue = ps2x.Analog(PSS_RY);
    RightYValue = constrain(ps2x.Analog(PSS_RY), 0, -50);
    SpeedSettingR = map(RightYValue, 135, 255, 0, -50);
    setMotorSpeed(RIGHT_MOTOR, SpeedSettingR);
    ps2x.read_gamepad();
  }

  void Both_MotorsF() 
  {
    setMotorDirection(BOTH_MOTORS, MOTOR_DIR_FORWARD);
    RightYValue = ps2x.Analog(PSS_RY);
    RightYValue = constrain(ps2x.Analog(PSS_RY), 0, 50);
    SpeedSettingR = map(RightYValue, 115, 0, 0, 50);
    setMotorSpeed(BOTH_MOTORS, SpeedSettingR);
  }

  void Both_MotorsB() 
  {
    setMotorDirection(BOTH_MOTORS, MOTOR_DIR_BACKWARD);
    RightYValue = ps2x.Analog(PSS_RY);
    RightYValue = constrain(ps2x.Analog(PSS_RY), 0, -50);
    SpeedSettingR = map(RightYValue, 135, 255, 0, -50);
    setMotorSpeed(BOTH_MOTORS, SpeedSettingR);
  }

  void Gripper() 
  {
    if (ps2x.ButtonPressed(PSB_CIRCLE)) 
    {
      myservo.write(0);
    } 
    else if (ps2x.ButtonPressed(PSB_TRIANGLE)) 
    {
      myservo.write(130);
    }
  }

  void Line_Following() 
  {
    while (ps2x.ButtonPressed(PSB_R1) == FALSE) 
    {
      uint32_t linePos = getLinePosition();
      if ((linePos > 0) && (linePos < 4000)) 
      {  // turn left
        setMotorSpeed(LEFT_MOTOR, 0);
        setMotorSpeed(RIGHT_MOTOR, 25);
      } 
      else if (linePos > 5000) 
      {  // turn right
        setMotorSpeed(LEFT_MOTOR, 25);
        setMotorSpeed(RIGHT_MOTOR, 0);
      } 
      else if (linePos == 0) 
      {
        enableMotor(2);
        setMotorDirection(BOTH_MOTORS, MOTOR_DIR_FORWARD);
        setMotorSpeed(BOTH_MOTORS, 0);
        STATE = Auto_Right;
      } 
      else 
      {  // go straight
        setMotorSpeed(LEFT_MOTOR, 25);
        setMotorSpeed(RIGHT_MOTOR, 25);
      }
    }
  }

  void Auto_Turn_Right() 
  {
    while (object <= 1) 
    {
      distIN = readSharpDistIN(1);      // Sensor block returns calculated distance in inches
      error = (distIN - stopDistance);  // how close are we?
      motorSpeed = abs(error) * 6;      // represents the control effort
      // amplifies the error. The number 6 is the P controller constant
      motorSpeed = constrain(motorSpeed, 0, 40);  //Motor speed can't be greater than 100 or less than zero
      readSharpDistIN(1);
      setMotorDirection(BOTH_MOTORS, MOTOR_DIR_FORWARD);
      delay(100);
      if ((object == 0) && (distIN <= 5.5) && (distIN >= 0)) 
      {
        setMotorSpeed(RIGHT_MOTOR, 0);
        setMotorSpeed(LEFT_MOTOR, 25);
        delay(1000);
        setMotorSpeed(BOTH_MOTORS, 25);
        delay(300);
        object = 1;
        readSharpDistIN(1);
      } 
      else if ((object == 1) && (distIN <= 5.5) && (distIN >= 0)) 
      {
        myservo.write(0);
        setMotorSpeed(BOTH_MOTORS, 0);
        delay(3000);
        object = 2;
        readSharpDistIN(1);
      } 
      else 
      {
        setMotorSpeed(BOTH_MOTORS, (int)motorSpeed);
        delayMicroseconds(100);
        readSharpDistIN(1);
      }
    }
  }

  void floorCalibration() 
  {
    /* Place Robot On Floor (no line) */
    delay(2000);
    UART_SERIAL.println("Push left button on Launchpad to begin calibration.");
    UART_SERIAL.println("Make sure the robot is on the floor away from the line.\n");
    /* Wait until button is pressed to start robot */
    waitBtnPressed(LP_LEFT_BTN, RED_LED);

    delay(500);
    UART_SERIAL.println("Running calibration on floor");

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
    waitBtnPressed(LP_LEFT_BTN, RED_LED);
    delay(1000);

    enableMotor(BOTH_MOTORS);
  }
