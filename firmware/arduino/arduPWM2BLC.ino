#include "include/BrushlessWheels.h"

#define MAX_RPM 100
#define PWM_PIN_L 6
#define PWM_PIN_R 7
#define PWM_L 1000
#define PWM_M 1500
#define PWM_H 2000

BrushlessWheels BW;

float pwm2rpm(int pwm)
{
  pwm = constrain(pwm, PWM_L, PWM_H);
  float rpm;

  if(pwm < PWM_M)
    rpm = (float)(pwm-PWM_M)/(float)(PWM_M-PWM_L);
  else
    rpm = (float)(pwm-PWM_M)/(float)(PWM_H-PWM_M);

  return rpm * (float)MAX_RPM;
}

void setup()
{
  pinMode(PWM_PIN_L, INPUT);
  pinMode(PWM_PIN_R, INPUT);

  Serial.begin(57600);
  BW.Init();
}

void loop()
{
  float rpmL = pwm2rpm(pulseIn(PWM_PIN_L, HIGH));
  float rpmR = pwm2rpm(pulseIn(PWM_PIN_R, HIGH));

  BW.DriveWheels(rpmL, rpmR); 
}
