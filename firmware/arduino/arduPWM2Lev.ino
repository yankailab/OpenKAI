#include <Arduino.h>
#include <Servo.h>

//#define SERIAL_DEBUG

#define PWM_IN_L 7
#define PWM_IN_R 8

#define PWM_OUT_L_LEV 7
#define PWM_OUT_L_REV 7
#define PWM_OUT_R_LEV 7
#define PWM_OUT_R_REV 7

#define PWM_L 800
#define PWM_M 1500
#define PWM_H 2200
#define PWM_DZ 10

Servo servoL;
Servo servoR;
Servo servoLrev;
Servo servoRrev;

void pwm2LR(int pwm, int* pLev, int* pRev)
{
  if (pwm < PWM_L || pwm > PWM_H)
  {
    *pLev = PWM_L;
    *pRev = PWM_H;
  }

  int pwmD = abs(pwm - PWM_M);

  if (pwmD < PWM_DZ)
  {  
    *pLev = PWM_L;
    *pRev = PWM_H;
  }
    
  *pLev = PWM_L + pwmD*2;

  if (pwm >= PWM_M)
    *pRev = PWM_H;
  else
    *pRev = PWM_L;
}

void setup()
{
  pinMode(PWM_IN_L, INPUT);
  pinMode(PWM_IN_R, INPUT);
  
  servoL.attach(PWM_OUT_L_LEV);
  servoLrev.attach(PWM_OUT_L_REV);

  servoR.attach(PWM_OUT_R_LEV);
  servoRrev.attach(PWM_OUT_R_REV);

#ifdef SERIAL_DEBUG
  Serial.begin(115200);
#endif  
}

void loop()
{
  int pwmL = pulseIn(PWM_IN_L, HIGH);
  int pwmR = pulseIn(PWM_IN_R, HIGH);

  int Llev;
  int Lrev;
  pwm2LR(pwmL, &Llev, &Lrev);  
  servoL.writeMicroseconds(Llev);
  servoLrev.writeMicroseconds(Lrev);

  int Rlev;
  int Rrev;
  pwm2LR(pwmR, &Rlev, &Rrev);  
  servoR.writeMicroseconds(Rlev);
  servoRrev.writeMicroseconds(Rrev);

#ifdef SERIAL_DEBUG
  Serial.print("pwmL: ");
  Serial.print(pwmL);
  Serial.print(", ");
  Serial.print(Llev);
  Serial.print(", ");
  Serial.print(Lrev);
  
  Serial.print("; pwmR: ");
  Serial.print(pwmR);
  Serial.print(", ");
  Serial.print(Rlev);
  Serial.print(", ");
  Serial.print(Rrev);

  Serial.println();
#endif

}