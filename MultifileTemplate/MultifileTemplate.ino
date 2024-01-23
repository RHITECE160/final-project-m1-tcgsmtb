#include "PS2X_lib.h"  //for v1.6
#include "SimpleRSLK.h"
#include <Servo.h>
#include <TinyIRremote.h>

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
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define IR_RCV_PIN      33

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
IRreceiver irRX(IR_RCV_PIN);
IRData IRresults;

void setup() 
{
  UART_SERIAL.begin(115200);
  myservo.attach(SRV_0);
  Serial.begin(57600);  //ZJE: changed from Arduino deafult of 9600
  setupRSLK();
  pinMode(LP_LEFT_BTN, INPUT_PULLUP);
  // delayMicroseconds(500 * 1000);  //added delay to give wireless ps2 module some time to startup, before configuring it
  // error = 1;
  // while (error) 
  // {
  //   error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT);
  //   if (error == 0) 
  //   {
  //     Serial.print("Found Controller, configured successful ");
  //   } 
  //   else if (error == 1)
  //     Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");

  //   else if (error == 2)
  //     Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  //   else if (error == 3)
  //     Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
  //   delayMicroseconds(1000 * 1000);
  // }
  // setupWaitBtn(LP_LEFT_BTN);
  // setupLed(RED_LED);
  // setupLed(GREEN_LED);

  // //  Serial.print(ps2x.Analog(1), HEX);

  // type = ps2x.readType();
  // switch (type) {
  //   case 0:
  //     Serial.print("Unknown Controller type found ");
  //     break;
  //   case 1:
  //     Serial.print("DualShock Controller found ");
  //     break;
  //   case 2:
  //     Serial.print("GuitarHero Controller found ");
  //     break;
  //   case 3:
  //     Serial.print("Wireless Sony DualShock Controller found ");
  //     break;
  // }

  if (irRX.initIRReceiver()) {
    Serial.println(F("Ready to receive NEC IR signals at pin " STR(IR_RCV_PIN)));
  } else {
    Serial.println("Initialization of IR receiver failed!");
    while (1) { ; }
  }
  enableRXLEDFeedback(BLUE_LED);
}




void loop() 
{
  ps2x.read_gamepad();

<<<<<<< HEAD
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
=======
  // if(ps2x.Button(PSB_PAD_UP)) 
  // {      
  //   Serial.print("Go Forward");
  //   forward();
  // }
  // if(ps2x.Button(PSB_PAD_RIGHT))
  // {
  //   Serial.print("Turn Right");
  //   turnRight();
  // }
  // if(ps2x.Button(PSB_PAD_LEFT))
  // {
  //   Serial.print("Turn Left");
  //   turnLeft();
  // }
  // if(ps2x.Button(PSB_PAD_DOWN))
  // {
  //   Serial.print("Go Backward");
  //   backward();
  // }   
  // if(ps2x.Button(PSB_TRIANGLE))
  // {
  //   Serial.print("Spin");
  //   spin();
  // }
  // if(ps2x.Button(PSB_CROSS))
  // {
  //   Serial.print("Stop");
  //   stop();
  // }




    if (irRX.decodeIR(&IRresults)) {
    Serial.print("A=0x");
    Serial.print(IRresults.address, HEX);
    Serial.print(" C=0x");
    Serial.print(IRresults.command, HEX);
    Serial.print(" is repeat: ");
    Serial.print(IRresults.isRepeat ? "TRUE" : "FALSE");
    Serial.println();
    translateIR();
>>>>>>> 3d9f9b9a8956e75352b96dc3d020e12e9613ba6d
  }
}

void translateIR() {
    switch (IRresults.command) {
    case 0x45:
      Serial.println("POWER");
      break;
    case 0x46:
      Serial.println("VOL+");
      forward();
      break;
    case 0x47:
      Serial.println("FUNC");
      break;
    case 0x44:
      Serial.println("LEFT");
      turnLeft();
      break;
    case 0x40:
      Serial.println("PLAY");
      stop();
      break;
    case 0x43:
      Serial.println("RIGHT");
      turnRight();
      break;
    case 0x9:
      Serial.println("UP");
      break;
    case 0x15:
      Serial.println("VOL-");
      backward();
      break;
    case 0x7:
      Serial.println("DOWN");
      break;
    case 0x16:
      Serial.println("0");
      break;
    case 0x19:
      Serial.println("EQ");
      break;
    case 0xD:
      Serial.println("ST");
      break;
    case 0xC:
      Serial.println("1");
      spin();
      break;
    case 0x18:
      Serial.println("2");
      break;
    case 0x5E:
      Serial.println("3");
      break;
    case 0x8:
      Serial.println("4");
      break;
    case 0x1C:
      Serial.println("5");
      break;
    case 0x5A:
      Serial.println("6");
      break;
    case 0x42:
      Serial.println("7");
      break;
    case 0x52:
      Serial.println("8");
      break;
    case 0x4A:
      Serial.println("9");
      break;
    default:
      Serial.println("other button");
      break;
    }
}

