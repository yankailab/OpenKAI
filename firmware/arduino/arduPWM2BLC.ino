#include "BrushlessWheels.h"

//#define SERIAL_DEBUG

#define MAX_RPM 100
#define PWM_PIN_L 7
#define PWM_PIN_R 8
#define PWM_L 1100
#define PWM_M 1500
#define PWM_H 1900
#define PWM_DZ 10

BrushlessWheels BW;

float pwm2rpm(int pwm)
{
  pwm = constrain(pwm, PWM_L, PWM_H);
  float rpm;

  if(abs(pwm-PWM_M) < PWM_DZ)
    rpm = 0.0;
  else if(pwm < PWM_M)
    rpm = (float)(pwm-PWM_M)/(float)(PWM_M-PWM_L);
  else
    rpm = (float)(pwm-PWM_M)/(float)(PWM_H-PWM_M);

  return rpm * (float)MAX_RPM;
}

void setup()
{
  pinMode(PWM_PIN_L, INPUT);
  pinMode(PWM_PIN_R, INPUT);

  BW.Init();

#ifdef SERIAL_DEBUG
  Serial.begin(115200);
#endif  
}

void loop()
{
  float rpmL = pwm2rpm(pulseIn(PWM_PIN_L, HIGH));
  float rpmR = pwm2rpm(pulseIn(PWM_PIN_R, HIGH));

  BW.DriveWheels(rpmL, rpmR);

#ifdef SERIAL_DEBUG
  Serial.print("rpm: ");
  Serial.print(rpmL);
  Serial.print(", ");
  Serial.print(rpmR);
  Serial.println();
#endif

}
