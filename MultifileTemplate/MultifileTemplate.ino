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
  ps2x.read_gamepad();

  if(ps2x.Button(PSB_PAD_UP)) 
  {      
    Serial.print("Go Forward");
    forward();
  }
  if(ps2x.Button(PSB_PAD_RIGHT))
  {
    Serial.print("Turn Right");
    turnRight();
  }
  if(ps2x.Button(PSB_PAD_LEFT))
  {
    Serial.print("Turn Left");
    turnLeft();
  }
  if(ps2x.Button(PSB_PAD_DOWN))
  {
    Serial.print("Go Backward");
    backward();
  }   
  if(ps2x.Button(PSB_TRIANGLE))
  {
    Serial.print("Spin");
    spin();
  }
  if(ps2x.Button(PSB_CROSS))
  {
    Serial.print("Stop");
    stop();
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
