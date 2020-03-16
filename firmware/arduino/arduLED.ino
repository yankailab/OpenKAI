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
}

#define N_IO 4
PIN_OI g_pinIO[N_IO];

void setup()
{
	g_pinIO.init(15,6);
	g_pinIO.init(14,7);
	g_pinIO.init(16,8);
	g_pinIO.init(10,9);
}

void loop()
{
	for(int i=0; i<N_IO; i++)
		g_pinIO[i].update();
}
