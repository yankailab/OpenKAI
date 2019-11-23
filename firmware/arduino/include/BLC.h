/**************************************************************************
 * A simple library to communicate and send command to Brushless Wheels UGV
 * by Rasheed Kittinanthapanya
 ***************************************************************************/

#ifndef BrushlessWheels_H
#define BrushlessWheels_H

#include "Arduino.h"

class BrushlessWheels
{
public:

	//class constructor to initialize baudrate setting
	BrushlessWheels()
	{
		Serial1.begin(9600);     // Specific baudrate for communication with ESC
	}

	// Init: initialize by do waitUntilFourByte, ESCHandShake and zeroSpeed
	// MUST DO inside the setup() loop on your sketch
	void Init()
	{
		waitUntilFourZero();         // Wait ESC to response and do a hand-shake
		delay(219);                // DON'T change this delay, it's from hacking
		ESCHandShake();

		for (i = 1; i < 10; i++)  // start the wheels from zero rpm before drive
		{
			zeroSpeed();
		}
	}

	// Int16ToByteData: split an unsigned int into 2 bytes data
	// unsigned int stores 16 bits as an integer
	// Pass by reference using array StoreByte[2], this is output of the function
	void Int16ToByteData(unsigned int Data, unsigned char StoreByte[2])
	{
		// unsigned int can store 16 bit int
		StoreByte[0] = (Data & 0xFF00) >> 8;      //High byte, most right of HEX
		StoreByte[1] = (Data & 0x00FF);             //Low byte, most left of HEX
	}

	// RPMToRaw: convert rotation per minute value to raw value
	// input is a float number between -MaxRPM to MaxRPM, minus sign means reverse rotation
	// output is a value from 0 to 3200 for forward rotation and 32769 to 35968 for reverse rotation
	unsigned int RPMToRaw(float rpm)
	{
		int raw_int;
		unsigned int out_raw;

		// map rpm to raw value
		raw_int = (int) map(rpm, 0.0, 144.0, 0, 3200);

		// In case of negative number, shift mapped number from 32768 to 35968 (for 0.0 to -146.0)
		if (rpm < 0.0)
		{
			out_raw = 32768 + abs(raw_int);
		}
		// In case of positive number, use the mapped number for unsigned int type
		else
		{
			out_raw = raw_int;
		}

		return out_raw;
	}

	// waitUntilFourZero: when start, ESC will send four bytes of zero then wait for our controller to response
	// if our control received four bytes of zero, then it will allow next command to run after
	void waitUntilFourZero()
	{
		while (startTick == true)
		{

			while (Serial1.available() > 0)
			{

				ReadByte = Serial1.read();
				Reply[i] = ReadByte;
				i++;
				ReadOK = true;
				delayMicroseconds(2000); // This delay will wait for incoming byte to complete 6 bytes as

			}

			if (ReadOK == true)
			{

				if ((Reply[0] == 0) && (Reply[1] == 0) && (Reply[2] == 0)
						&& (Reply[3] == 0))
				{
					//Serial.println("Get start byte");
					startTick = false;
				}

				i = 0;    // reset i
				//memset(Reply, 0, sizeof(Reply));
				ReadOK = false;

			}
		}

	}

	// ESCHandShake: from hack, we need to send 17bytes of "Hand-Shake" style 20times with specific delay
	void ESCHandShake()
	{
		Serial.println("Here");
		for (i = 1; i <= 20; i++)
		{
			Serial1.write(0x01);
			Serial1.write(0x11);
			Serial1.write(0x28);
			Serial1.write(0x02);
			Serial1.write(0x50);
			Serial1.write(0x32);
			Serial1.write(0x03);
			Serial1.write(0x1E);
			Serial1.write(0x83);
			Serial1.write(0x15);
			Serial1.write(0x06);
			Serial1.write(0x0A);
			Serial1.write(0x01);
			Serial1.write(0x03);
			Serial1.write(0x04);
			Serial1.write(0x07);
			Serial1.write(0x96);
			Serial1.flush();

			if (i == 1)
			{
				delayMicroseconds(300); // DON'T change this delay, it's from hacking
			}
			else
			{

				delay(14);         // DON'T change this delay, it's from hacking
			}
		}
	}

	// zeroSpeed: set motor to zero rpm, this will be used when start after hand shake
	void zeroSpeed()
	{
		Serial1.write(Header1);
		Serial1.write(Header2);
		Serial1.write(0x00);            // Motor1 speed hibyte
		Serial1.write(0x00);            // Motor1 speed lobyte
		Serial1.write(0x00);            // Motor2 speed hibyte
		Serial1.write(0x00);            // Motor2 speed lobyte
		Serial1.write(0xB4);            // Mode hibyte (don't care)
		Serial1.write(0x00);            // Mode lobyte (don't care)
		Serial1.write(0xBF);            // Check sum
		Serial.flush();
		delay(23);                 // DON'T change this delay, it's from hacking

	}

	// DriveWheels: drive both wheels with desired rpm, + for forward - for reverse rotations
	void DriveWheels(float rpm1, float rpm2)
	{

		RawInt1 = RPMToRaw(rpm1);
		RawInt2 = RPMToRaw(rpm2);
		Int16ToByteData(RawInt1, Motor1SpeedByte);
		Int16ToByteData(RawInt2, Motor2SpeedByte);

		byte Motor1hibyte = Motor1SpeedByte[0];
		byte Motor1lobyte = Motor1SpeedByte[1];

		byte Motor2hibyte = Motor2SpeedByte[0];
		byte Motor2lobyte = Motor2SpeedByte[1];

		byte Modehibyte = 0x00;    // don't care
		byte Modelobyte = 0x00;    // don't care
		byte CheckSum = Header1 + Header2 + Motor1hibyte + Motor1lobyte
				+ Motor2hibyte + Motor2lobyte + Modehibyte + Modelobyte;

		Serial1.write(Header1);
		Serial1.write(Header2);
		Serial1.write(Motor1hibyte);
		Serial1.write(Motor1lobyte);
		Serial1.write(Motor2hibyte);
		Serial1.write(Motor2lobyte);
		Serial1.write(Modehibyte);
		Serial1.write(Modelobyte);
		Serial1.write(CheckSum);
		Serial.flush();
		delay(23);                 // DON'T change this delay, it's from hacking
	}

private:

	bool startTick = true;
	unsigned char ReadByte;
	unsigned char Reply[6];
	int count;
	int i = 0;
	bool ReadOK = false;

	float MaxRPM;
	unsigned int RawInt1;
	unsigned int RawInt2;
	unsigned char Motor1SpeedByte[2];
	unsigned char Motor2SpeedByte[2];

	byte Header1 = 0x02;
	byte Header2 = 0x09;

};

#endif
