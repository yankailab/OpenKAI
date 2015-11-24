#pragma once
#include "../Base/platform.h"

#include <sys/ioctl.h>
//#include <linux/serial.h>

#include <cstdlib>
#include <stdio.h>   // Standard input/output definitions
#include <unistd.h>  // UNIX standard function definitions
#include <fcntl.h>   // File control definitions
#include <termios.h> // POSIX terminal control definitions
#include <pthread.h> // This uses POSIX Threads
#include <signal.h>

// The following two non-standard baudrates should have been defined by the system
// If not, just fallback to number
#ifndef B460800
#define B460800 460800
#endif

#ifndef B921600
#define B921600 921600
#endif

#define ARDUINO_WAIT_TIME 2000

using namespace std;

class SerialPort
{
public:
	//Initialize Serial communication with the given COM port
	SerialPort();

	//Close the connection
	//NOTA: for some reason you can't connect again before exiting
	//the program and running it again
	~SerialPort();

	bool Open(char *portName);
	bool Setup(int baud, int data_bits, int stop_bits, bool parity, bool hardware_control);
	int  Read(char *buffer, unsigned int nbChar);
	void Write(char *buffer, unsigned int nbChar);
	void Close(void);

	//Check if we are actually connected
	bool IsConnected();

private:
//		void* m_pSerialPort;

	//Connection status
	bool bConnected;
	int  m_fd;

	string m_portName;
	int  m_baudrate;
	pthread_mutex_t m_portMutex;


};

