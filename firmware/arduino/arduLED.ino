#include <Arduino.h>

struct PIN_IO
{
  int m_in;
  int m_out;
  
  void init(int in, int out)
  {
    m_in = in;
    m_out = out;
    
    pinMode(m_in, INPUT);
      pinMode(m_out, OUTPUT);
  }
  
  void update(void)
  {
    digitalWrite(m_out, digitalRead(m_in));   
  }
};

#define N_IO 2
PIN_IO g_pinIO[N_IO];

void setup()
{
  g_pinIO[0].init(14,9);
  g_pinIO[1].init(15,7);
}

void loop()
{
  for(int i=0; i<N_IO; i++)
    g_pinIO[i].update();
}
