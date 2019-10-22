#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Servo.h>
#include "arduOK.h"

//#define SERIAL_DEBUG

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
#define PIN_LED_IDLE A0
#define PIN_LED_MANUAL A1
#define PIN_LED_AUTO A2
#define PIN_LED_INDICATOR A3

#define PIN_IN_STEER 4
#define PIN_IN_SPEED 7
#define PIN_IN_MODE 8
#define PIN_IN_ACTION 12

//----------------------------------------------------------------------
// Motor
#define PWM_MID 1500
#define PWM_MID_DZ 50
#define PWM_LOW 500
#define PWM_LIM 500

struct MOTOR
{
  Servo m_servo;
  uint16_t m_pwm;
  uint8_t m_pin;
  int8_t m_dir;
};

#define N_MOT 4
MOTOR g_pMot[N_MOT];

//----------------------------------------------------------------------
// PWM input from transmitter
uint16_t g_pwmMode;
uint16_t g_pwmAction;
uint16_t g_pwmSteer;
uint16_t g_pwmSpeed;

//----------------------------------------------------------------------
// Mode
#define MODE_IDLE 0
#define MODE_MANUAL 1
#define MODE_AUTO 2
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

void runCMD(void)
{
  switch (g_cmd.m_pBuf[1])
  {
    case ROVERCMD_PWM:
//      g_pwmLap = *((uint16_t*)(&m_cmd.m_pBuf[3]));
//      g_pwmRap = *((uint16_t*)(&m_cmd.m_pBuf[5]));
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
  for (i = 0; i < N_MOT; i++)
  {
    g_pMot[i].m_servo.writeMicroseconds(g_pMot[i].m_pwm);
  }
}

void mode()
{
  if (g_pwmMode <= PWM_LOW)// || g_pwmLin <= PWM_LOW || g_pwmRin <= PWM_LOW)
  {
    g_Mode = MODE_IDLE;
    return;
  }

  if (abs(g_pwmMode - PWM_MID) < PWM_MID_DZ)
  {
    g_Mode = MODE_MANUAL;
    return;
  }

  if (g_tNow - g_tAP > AP_TIMEOUT)
  {
    g_Mode = MODE_IDLE;
    return;
  }

//  if (g_pwmMode > PWM_MID)
//    g_Mode = MODE_FORWARD;
//  else
//    g_Mode = MODE_BACKWARD;

}

void updateLED(void)
{
  digitalWrite(PIN_LED_IDLE, LOW);
  digitalWrite(PIN_LED_MANUAL, LOW);
  digitalWrite(PIN_LED_AUTO, LOW);

  if (g_Mode == MODE_IDLE)
  {
    digitalWrite(PIN_LED_IDLE, HIGH);
    digitalWrite(PIN_LED_INDICATOR, LOW);
  }
  else if (g_Mode == MODE_MANUAL)
  {
    digitalWrite(PIN_LED_MANUAL, HIGH);
    digitalWrite(PIN_LED_INDICATOR, LOW);
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
  g_pMot[0].m_pin = 3;
  g_pMot[1].m_pin = 5;
  g_pMot[2].m_pin = 6;
  g_pMot[3].m_pin = 9;

  g_pMot[0].m_dir = 1;
  g_pMot[1].m_dir = 1;
  g_pMot[2].m_dir = 1;
  g_pMot[3].m_dir = 1;


  for (int i = 0; i < N_MOT; i++)
  {
    MOTOR* pM = &g_pMot[i];
    pinMode(pM->m_pin, OUTPUT);
    pM->m_pwm = PWM_MID;
    pM->m_servo.attach(pM->m_pin);
  }
  updateMotor();

  g_tAP = 0;
  g_Mode = MODE_IDLE;

  pinMode(PIN_LED_IDLE, OUTPUT);
  pinMode(PIN_LED_MANUAL, OUTPUT);
  pinMode(PIN_LED_AUTO, OUTPUT);
  pinMode(PIN_LED_INDICATOR, OUTPUT);

  pinMode(PIN_IN_MODE, INPUT);
  pinMode(PIN_IN_ACTION, INPUT);
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
  g_pwmAction = pulseIn(PIN_IN_ACTION, HIGH, PWM_TIMEOUT);

  g_pMot[0].m_pwm = g_pwmSteer;
  g_pMot[1].m_pwm = g_pwmSpeed;
  g_pMot[2].m_pwm = g_pwmMode;
  g_pMot[3].m_pwm = g_pwmAction;

  updateMotor();

//  g_UDP.parsePacket();
//  decodeCMD();

//  g_UDP.beginPacket(g_UDP.remoteIP(), g_UDP.remotePort());
//  g_UDP.write(PROTOCOL_BEGIN);            //start mark
//  g_UDP.write((uint8_t)ROVERCMD_STATE);  //cmd
//  g_UDP.write(4);                         //payload len
//  g_UDP.write((uint8_t)(g_pwm1 & 0xFF));
//  g_UDP.write((uint8_t)((g_pwm1 >> 8) & 0xFF));
//  g_UDP.write((uint8_t)(g_pwm2 & 0xFF));
//  g_UDP.write((uint8_t)((g_pwm2 >> 8) & 0xFF));
//  g_UDP.endPacket();

#ifdef SERIAL_DEBUG
    Serial.print("pwmSteer=");
    Serial.print(g_pwmSteer);
    Serial.print(", pwmSpeed=");
    Serial.print(g_pwmSpeed);
    Serial.print(", pwmMode=");
    Serial.print(g_pwmMode);
    Serial.print(", pwmAction=");
    Serial.println(g_pwmAction);
#endif

}