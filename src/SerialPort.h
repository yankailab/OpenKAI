#pragma once
#include "platform.h"

#include <sys/ioctl.h>
//#include <linux/serial.h>

#define ARDUINO_WAIT_TIME 2000

class SerialPort
{
    private:
		void* m_pSerialPort;


		//Connection status
		bool bConnected;
		int m_fd;
	

	public:
        //Initialize Serial communication with the given COM port
        SerialPort();

        //Close the connection
        //NOTA: for some reason you can't connect again before exiting
        //the program and running it again
        ~SerialPort();

		bool connect(char *portName);

		//Read data in a buffer, if nbChar is greater than the
        //maximum number of bytes available, it will return only the
        //bytes available. The function return -1 when nothing could
        //be read, the number of bytes actually read.
        int ReadData(char *buffer, unsigned int nbChar);
        
		//Writes data from a buffer through the Serial connection
        //return true on success.
        bool WriteData(char *buffer, unsigned int nbChar);
        
		//Check if we are actually connected
        bool IsConnected();

};

