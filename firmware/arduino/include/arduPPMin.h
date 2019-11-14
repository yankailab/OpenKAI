#ifndef PPMReader_h
#define PPMReader_h

#include "Arduino.h"

#define PPM_INPUT_N_CHANNEL 16
#define CPU_SPEED_MULTIPLIER (F_CPU/8000000)
#define PPM_INPUT_NO_SIGNAL_T 500 //if no update in this number of ms, raise alarm

#define PPM_INPUT_PULSE 510
#define PPM_INPUT_SYNC 1910
#define PPM_INPUT_MIN 810
#define PPM_INPUT_MAX 2210

class PPMReader
{
public:
	volatile static int s_ppm[PPM_INPUT_N_CHANNEL];
  volatile static bool s_bPPMbUseTimer;
  volatile static uint32_t s_tPPMlastPacket;

	static void handler()
	{
		static unsigned int pulse;
		static unsigned long counter;
		static byte channel;
		static unsigned long previousCounter = 0;
		static unsigned long currentMicros = 0;
		int tmpVal;

		if (s_bPPMbUseTimer)
		{
			counter = TCNT1 * CPU_SPEED_MULTIPLIER;
			TCNT1 = 0;
		}
		else
		{
			currentMicros = micros();
			counter = currentMicros - previousCounter;
			previousCounter = currentMicros;
		}

		if (counter < PPM_INPUT_PULSE)
		{ //must be a pulse
			pulse = counter;
		}
		else if (counter > PPM_INPUT_SYNC)
		{ //sync
			channel = 0;
			s_tPPMlastPacket = millis();
		}
		else
		{ //servo values between 810 and 2210 will end up here
			tmpVal = counter + pulse;
			if (tmpVal > PPM_INPUT_MIN && tmpVal < PPM_INPUT_MAX)
			{
				s_ppm[channel] = tmpVal;
			}
			channel++;
		}
	}

	PPMReader(int pin, int interrupt, bool bUseTimer = false)
	{
		_pin = pin;
		_interrupt = interrupt;
		s_bPPMbUseTimer = bUseTimer;
    s_tPPMlastPacket = 0;

		pinMode(_pin, INPUT);
		attachInterrupt(_interrupt, PPMReader::handler, CHANGE);
	}

	int get(uint8_t channel)
	{
		return s_ppm[channel];
	}

	bool isReceiving(void)
	{
		if (millis() - s_tPPMlastPacket > PPM_INPUT_NO_SIGNAL_T)
			return false;

		return true;
	}

	void start(void)
	{
		attachInterrupt(_interrupt, PPMReader::handler, CHANGE);
	}

	void stop(void)
	{
		detachInterrupt(_interrupt);
	}

private:
	int _pin;
	int _interrupt;
};

#endif
