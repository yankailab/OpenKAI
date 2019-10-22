#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Servo.h>
#include "arduOK.h"

#define SERIAL_DEBUG

//----------------------------------------------------------------------
// IO override for Ethernet
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress g_myIP(192, 168, 8, 250);
unsigned int g_myPort = 8888;
EthernetUDP g_UDP;

bool ioAvailable(void)
{
  return g_UDP.available();
}

byte ioRead(void)
{
  return g_UDP.read();
}

//----------------------------------------------------------------------
// Pin assign
#define PIN_LED_STOP A0
#define PIN_LED_MANUAL A1
#define PIN_LED_AUTO A2
#define PIN_LED_INDICATOR A3

#define PIN_IN_STEER 4
#define PIN_IN_SPEED 7
#define PIN_IN_MODE 8

//----------------------------------------------------------------------
// Motor
#define PWM_MID 1500
#define PWM_D 500
#define PWM_MID_DZ 50
#define PWM_LOWLIM 600

struct MOTOR
{
  Servo m_servo;
  int16_t m_pwm;
  int16_t m_pwmL;
  int16_t m_pwmM;
  int16_t m_pwmH;
  int8_t m_pin;
  int8_t m_dir;

  void init(int16_t pwmM, int16_t pwmL, int16_t pwmH, int8_t pin, int8_t dir)
  {
    m_pwmL = pwmL;
    m_pwmM = pwmM;
    m_pwmH = pwmH;
    m_pwm = m_pwmM;
    m_pin = pin;
    m_dir = dir;
    
    pinMode(m_pin, OUTPUT);
    m_servo.attach(m_pin);
    m_servo.writeMicroseconds(m_pwm);
  }

  void speed(float s)
  {
    float d = constrain(s, -1.0, 1.0);

    if(d >= 0.0)
      m_pwm = d * ((float)(m_pwmH - m_pwmM)*m_dir);
    else
      m_pwm = d * ((float)(m_pwmM - m_pwmL)*m_dir);

    m_servo.writeMicroseconds(m_pwm);
  }  
};

#define N_MOT 4
MOTOR g_pMot[N_MOT];
float g_pMotAP[N_MOT];

//----------------------------------------------------------------------
// PWM input from transmitter
uint16_t g_pwmMode;
uint16_t g_pwmSteer;
uint16_t g_pwmSpeed;

float g_dirSpeed;
float g_dirSteer;

//----------------------------------------------------------------------
// Mode
#define MODE_STOP 0
#define MODE_MANUAL 1
#define MODE_AUTO_1 2
#define MODE_AUTO_2 3
uint8_t g_Mode;

#define AP_TIMEOUT 1000
#define PWM_TIMEOUT 100000
long g_tAP;
long g_tNow;

//----------------------------------------------------------------------
// CMD Protocol
#define ROVERCMD_STATE 0
#define ROVERCMD_PWM 1
#define ROVERCMD_PINOUT 2

#define FLT_SCALE 1000
#define FLT_SCALE_INV 0.001

void runCMD(void)
{
  switch (g_cmd.m_pBuf[1])
  {
    case ROVERCMD_PWM:
      g_pMotAP[0] = ((float)(*((uint16_t*)(&g_cmd.m_pBuf[3]))))*FLT_SCALE_INV;
      g_pMotAP[1] = ((float)(*((uint16_t*)(&g_cmd.m_pBuf[5]))))*FLT_SCALE_INV;
      g_pMotAP[2] = ((float)(*((uint16_t*)(&g_cmd.m_pBuf[6]))))*FLT_SCALE_INV;
      g_pMotAP[3] = ((float)(*((uint16_t*)(&g_cmd.m_pBuf[7]))))*FLT_SCALE_INV;
      g_tAP = millis();
      break;

    case ROVERCMD_PINOUT:
      digitalWrite(g_cmd.m_pBuf[3], g_cmd.m_pBuf[4]);
      g_tAP = millis();
      break;

    default:
      break;
  }
}

//----------------------------------------------------------------------
// Update
void updateMotor(void)
{
  int i;
  
  if (g_Mode == MODE_STOP)
  {
    for(i=0; i<N_MOT; i++)
      g_pMot[i].speed(0.0);
  }
  else if (g_Mode == MODE_MANUAL)
  {
    float nSpeed = 0.0;
    float pwm = g_pwmSpeed - PWM_MID;
    if(abs(pwm) > PWM_MID_DZ)
      nSpeed = (pwm / (float)PWM_D) * g_dirSpeed;

    float dSpeed = 0.0;
    pwm = g_pwmSteer - PWM_MID;
    if(abs(pwm) > PWM_MID_DZ)
      dSpeed = (pwm / (float)PWM_D) * g_dirSteer;
      
    g_pMot[0].speed(nSpeed + dSpeed);  //LF
    g_pMot[1].speed(nSpeed - dSpeed);  //RB
    g_pMot[2].speed(nSpeed - dSpeed);  //RF
    g_pMot[3].speed(nSpeed + dSpeed);  //LB

  }
  else //AUTO
  {
    for(i=0; i<N_MOT; i++)
      g_pMot[i].speed(g_pMotAP[i]);
  }
}

void updateMode()
{
  if (g_pwmMode <= PWM_LOWLIM
      || g_pwmSteer <= PWM_LOWLIM
      || g_pwmSpeed <= PWM_LOWLIM)
  {
    g_Mode = MODE_STOP;
    return;
  }

  if (abs(g_pwmMode - PWM_MID) < PWM_MID_DZ)
  {
    g_Mode = MODE_MANUAL;
    return;
  }

  if (g_tNow - g_tAP > AP_TIMEOUT)
  {
    g_Mode = MODE_STOP;
    return;
  }

  if (g_pwmMode > PWM_MID)
    g_Mode = MODE_AUTO_1;
  else
    g_Mode = MODE_AUTO_2;

}

void updateLED(void)
{
  digitalWrite(PIN_LED_STOP, LOW);
  digitalWrite(PIN_LED_MANUAL, LOW);
  digitalWrite(PIN_LED_AUTO, LOW);

  if (g_Mode == MODE_STOP)
  {
    digitalWrite(PIN_LED_STOP, HIGH);
  }
  else if (g_Mode == MODE_MANUAL)
  {
    digitalWrite(PIN_LED_MANUAL, HIGH);
  }
  else
  {
    digitalWrite(PIN_LED_AUTO, HIGH);
  }
}


//----------------------------------------------------------------------
// Setup

void setup()
{
  g_pMot[0].init(PWM_MID, PWM_MID-PWM_D, PWM_MID+PWM_D, 3, 1);  //LF
  g_pMot[1].init(PWM_MID, PWM_MID-PWM_D, PWM_MID+PWM_D, 5, -1);  //RB
  g_pMot[2].init(PWM_MID, PWM_MID-PWM_D, PWM_MID+PWM_D, 6, -1);  //RF
  g_pMot[3].init(PWM_MID, PWM_MID-PWM_D, PWM_MID+PWM_D, 9, 1);  //LB

  g_pMotAP[0] = 0.0;
  g_pMotAP[1] = 0.0;
  g_pMotAP[2] = 0.0;
  g_pMotAP[3] = 0.0;

  g_dirSpeed = 1.0;
  g_dirSteer = 1.0;

  g_tAP = 0;
  g_Mode = MODE_STOP;

  pinMode(PIN_LED_STOP, OUTPUT);
  pinMode(PIN_LED_MANUAL, OUTPUT);
  pinMode(PIN_LED_AUTO, OUTPUT);
  pinMode(PIN_LED_INDICATOR, OUTPUT);

  pinMode(PIN_IN_MODE, INPUT);
  pinMode(PIN_IN_STEER, INPUT);
  pinMode(PIN_IN_SPEED, INPUT);

  Serial.begin(115200);

  // configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields

  while (1)
  {
    Ethernet.begin(mac, g_myIP);

    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.println("Ethernet device not found");
      continue;
    }

    if (Ethernet.hardwareStatus() == EthernetW5100)
      Serial.println("W5100");
    else if (Ethernet.hardwareStatus() == EthernetW5200)
      Serial.println("W5200");
    else if (Ethernet.hardwareStatus() == EthernetW5500)
      Serial.println("W5500");
    else
      Serial.println("Unknown Ethernet");

    if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.println("Ethernet cable is not connected.");
      //      continue;
    }

    break;
  }

  g_UDP.begin(g_myPort);
  g_nMsg = 1;
}

//----------------------------------------------------------------------
// Loop
void loop()
{
  g_pwmSteer = pulseIn(PIN_IN_STEER, HIGH, PWM_TIMEOUT);
  g_pwmSpeed = pulseIn(PIN_IN_SPEED, HIGH, PWM_TIMEOUT);
  g_pwmMode = pulseIn(PIN_IN_MODE, HIGH, PWM_TIMEOUT);

  updateMode();
  updateMotor();
  updateLED();

  g_UDP.parsePacket();
  decodeCMD();

  g_UDP.beginPacket(g_UDP.remoteIP(), g_UDP.remotePort());
  g_UDP.write(PROTOCOL_BEGIN);            //start mark
  g_UDP.write((uint8_t)ROVERCMD_STATE);   //cmd
  g_UDP.write(1);                         //payload len
  g_UDP.write(g_Mode);
  g_UDP.endPacket();

  //  g_UDP.write((uint8_t)(g_pwm2 & 0xFF));
  //  g_UDP.write((uint8_t)((g_pwm2 >> 8) & 0xFF));


#ifdef SERIAL_DEBUG
  Serial.print("Steer=");
  Serial.print(g_pwmSteer);
  Serial.print(", Speed=");
  Serial.print(g_pwmSpeed);
  Serial.print(", Mode=");
  Serial.print(g_pwmMode);
#endif

}