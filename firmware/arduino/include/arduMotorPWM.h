#ifndef MOT_PWM_h
#define MOT_PWM_h
#include <Servo.h>

#define PWM_MID 1500
#define PWM_D 500
#define PWM_MID_DZ 25
#define PWM_MID_DZ_BTN 100
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

    if (d >= 0.0)
      m_pwm = PWM_MID + d * ((float)(m_pwmH - m_pwmM) * m_dir);
    else
      m_pwm = PWM_MID + d * ((float)(m_pwmM - m_pwmL) * m_dir);

    m_pwm = constrain(m_pwm, 1400, 1600);

    m_servo.writeMicroseconds(m_pwm);
  }
};

#endif
