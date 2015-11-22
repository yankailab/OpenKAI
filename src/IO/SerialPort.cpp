#include "SerialPort.h"

#ifdef PLATFORM_WIN
#include <Windows.h>

struct SERIAL_PORT
{
	//Serial comm handler
	HANDLE hSerial;
	//Get various information about the connection
	COMSTAT status;
	//Keep track of last error
	DWORD errors;

	//Connection status
	bool bConnected;

	//Serial comm file descriptor
//	int m_fd;
};

SERIAL_PORT g_serialPort;

SerialPort::SerialPort(void)
{
	this->m_pSerialPort = &g_serialPort;

	SERIAL_PORT* pSport = (SERIAL_PORT*)this->m_pSerialPort;
	pSport->bConnected = false;
}

bool SerialPort::IsConnected()
{
	SERIAL_PORT* pSport = (SERIAL_PORT*)this->m_pSerialPort;

	//Simply return the connection status
	return pSport->bConnected;
}

SerialPort::~SerialPort()
{
	SERIAL_PORT* pSport = (SERIAL_PORT*)this->m_pSerialPort;

	//Check if we are bConnected before trying to disconnect
	if(pSport->bConnected)
	{
		//We're no longer bConnected
		pSport->bConnected = false;
		//Close the serial handler
		CloseHandle(pSport->hSerial);
	}
}

bool SerialPort::connect(char *portName)
{
	this->m_pSerialPort = &g_serialPort;
	SERIAL_PORT* pSport = (SERIAL_PORT*)this->m_pSerialPort;

	pSport->bConnected = false;

	//Try to connect to the given port through CreateFile
	pSport->hSerial = CreateFile(portName,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			/*FILE_FLAG_OVERLAPPED,//*/FILE_ATTRIBUTE_NORMAL,
			NULL);

	//Check if the connection was successfull
	if (pSport->hSerial == INVALID_HANDLE_VALUE)
	{
		//If not success full display an Error
		if(GetLastError()==ERROR_FILE_NOT_FOUND)
		{
			//Print Error if neccessary
//            printf("ERROR: Handle was not attached. Reason: %s not available.\n", portName);

		}
		else
		{
			printf("ERROR!!!");
		}

		return false;
	}
	else
	{
		//If bConnected we try to set the comm parameters
		DCB dcb =
		{	0};

		//Try to get the current
		if (!GetCommState(pSport->hSerial, &dcb))
		{
			//If impossible, show an error
			printf("failed to get current serial parameters!");
			return false;
		}

		//Define serial connection parameters for the arduino board
		dcb.BaudRate = CBR_115200;//CBR_9600;
		dcb.ByteSize=8;
		dcb.StopBits=ONESTOPBIT;
		dcb.Parity=NOPARITY;
//		dcb.fDtrControl = DTR_CONTROL_ENABLE;
//		dcb.fRtsControl = RTS_CONTROL_ENABLE;
//		dcb.fOutxDsrFlow = true;
//		dcb.fOutxCtsFlow = true;
//		dcb.fAbortOnError = false;
		dcb.fNull = false;

//Set the parameters and check for their proper application
		if(!SetCommState(pSport->hSerial, &dcb))
		{
			printf("ALERT: Could not set Serial Port parameters");
			return false;
		}

		COMMTIMEOUTS timeouts;
		timeouts.ReadIntervalTimeout = 1;
		timeouts.ReadTotalTimeoutMultiplier = 1;
		timeouts.ReadTotalTimeoutConstant = 1;
		timeouts.WriteTotalTimeoutMultiplier = 1;
		timeouts.WriteTotalTimeoutConstant = 1;
		if (!SetCommTimeouts(pSport->hSerial, &timeouts))
		{
			printf("ALERT: Could not set Serial Port timeouts");
			return false;
		}

//	   	SetupComm(hSerial, 10240, 10240);

//If everything went fine we're bConnected
		pSport->bConnected = true;
		//We wait 2s as the arduino board will be reseting
		Sleep(ARDUINO_WAIT_TIME);
	}

	return true;
}

int SerialPort::ReadData(char *buffer, unsigned int nbChar)
{
	SERIAL_PORT* pSport = (SERIAL_PORT*)this->m_pSerialPort;

	//Number of bytes we'll have read
	DWORD bytesRead;
	//Number of bytes we'll really ask to read
	unsigned int toRead;

	//Use the ClearCommError function to get status info on the Serial port
	ClearCommError(pSport->hSerial, &pSport->errors, &pSport->status);

	//Check if there is something to read
	if (pSport->status.cbInQue > 0)
	{
		//If there is we check if there is enough data to read the required number
		//of characters, if not we'll read only the available characters to prevent
		//locking of the application.
		if (pSport->status.cbInQue > nbChar)
		{
			toRead = nbChar;
		}
		else
		{
			toRead = pSport->status.cbInQue;
		}

		//Try to read the require number of chars, and return the number of read bytes on success
		if (ReadFile(pSport->hSerial, buffer, toRead, &bytesRead, NULL) && bytesRead != 0)
		{
			return bytesRead;
		}
	}

	//If nothing has been read, or that an error was detected return -1
	return -1;

}

bool SerialPort::WriteData(char *buffer, unsigned int nbChar)
{
	SERIAL_PORT* pSport = (SERIAL_PORT*)this->m_pSerialPort;

	DWORD bytesSend;
	bool result;

	//Try to write the buffer on the Serial port
	result = WriteFile(pSport->hSerial, (void *)buffer, nbChar, &bytesSend, 0);

	if (bytesSend != nbChar)
	{
		PurgeComm(pSport->hSerial, PURGE_TXCLEAR | PURGE_TXABORT | PURGE_RXCLEAR | PURGE_RXABORT);
		return false;
	}
	else if(!result)
	{
		//In case it don't work get comm error and return false
		ClearCommError(pSport->hSerial, &pSport->errors, &pSport->status);
		return false;
	}

	return true;

}

#else
//for Linux

SerialPort::SerialPort(void)
{
	this->bConnected = false;
	m_fd = -1;

	m_portName = (char*) "/dev/ttyUSB0";
	m_baudrate = 57600;

	// Start mutex
	int result = pthread_mutex_init(&m_portMutex, NULL);
	if (result != 0)
	{
		printf("mutex init failed\n");
	}

}

bool SerialPort::IsConnected()
{
	//Simply return the connection status
	return this->bConnected;
}

SerialPort::~SerialPort()
{
	//Check if we are bConnected before trying to disconnect
	if (this->bConnected)
	{
		//We're no longer bConnected
		this->bConnected = false;
		//Close the serial file descriptor
		close(m_fd);
	}

	// destroy mutex
	pthread_mutex_destroy(&m_portMutex);
}

bool SerialPort::Open(char *portName)
{
	this->bConnected = false;

	m_portName = portName;

//	m_fd = open(m_portName.c_str(), O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);	// | O_NDELAY);
	m_fd = open(m_portName.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
	if (m_fd == -1)
	{
//		printf("init_serialport: Unable to open port\n");
		return false;
	}

	fcntl(m_fd, F_SETFL, 0);

//	usleep(2000 * 1000);
	this->bConnected = true;

	return true;
}

void SerialPort::Close(void)
{
	if (close(m_fd))
	{
		printf("WARNING: Error on port close (%i)\n");
	}
}


bool SerialPort::Setup(int baud, int data_bits, int stop_bits, bool parity,
		bool hardware_control)
{
	m_baudrate = baud;

	// Check file descriptor
	if (!isatty(m_fd))
	{
		printf("ERROR: file descriptor %d is NOT a serial port\n");
		return false;
	}

	// Read file descritor configuration
	struct termios config;
	if (tcgetattr(m_fd, &config) < 0)
	{
		printf("\nERROR: could not read configuration of fd %d\n");
		return false;
	}

	// Input flags - Turn off input processing
	// convert break to null byte, no CR to NL translation,
	// no NL to CR translation, don't mark parity errors or breaks
	// no input parity check, don't strip high bit off,
	// no XON/XOFF software flow control
	config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);
//	config.c_iflag &= ~(IXON | IXOFF | IXANY); // | BRKINT | ICRNL | INPCK | ISTRIP | IXON | IUCLC | INLCR| IXANY); // turn off s/w flow ctrl
//	config.c_iflag |= IGNPAR;


	// Output flags - Turn off output processing
	// no CR to NL translation, no NL to CR-NL translation,
	// no NL to CR translation, no column 0 CR suppression,
	// no Ctrl-D suppression, no fill characters, no case mapping,
	// no local output processing
	config.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR | OFILL | OPOST);
//	toptions.c_oflag = 0;
//  toptions.c_oflag &= ~(OPOST|OLCUC|ONLCR|OCRNL|ONLRET|OFDEL); // make raw


#ifdef OLCUC
	config.c_oflag &= ~OLCUC;
#endif

#ifdef ONOEOT
	config.c_oflag &= ~ONOEOT;
#endif


	// No line processing:
	// echo off, echo newline off, canonical mode off,
	// extended input processing off, signal chars off
	config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
//	config.c_lflag = 0;
//	toptions.c_lflag = 0;
//  toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN); // make raw

	// Turn off character processing
	// clear current char size mask, no parity checking,
	// no output processing, force 8 bit input
	config.c_cflag &= ~(CSIZE | PARENB);
	config.c_cflag |= CS8;

	// 8N1
/*	config.c_cflag &= ~CSTOPB;
	// no flow control
	config.c_cflag &= ~CRTSCTS;
	//    toptions.c_cflag |= CRTSCTS;
*/
//	config.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines


	// One input byte is enough to return from read()
	// Inter-character timer off
	config.c_cc[VMIN] = 0;
	config.c_cc[VTIME] = 10; // was 0
	// see: http://unixwiz.net/techtips/termios-vmin-vtime.html
//	toptions.c_cc[VMIN] = 0;
//	toptions.c_cc[VTIME] = 10;


	// Apply baudrate
	switch (baud)
	{
	case 1200:
		if (cfsetispeed(&config, B1200) < 0 || cfsetospeed(&config, B1200) < 0)
		{
			fprintf(stderr,
					"\nERROR: Could not set desired baud rate of %d Baud\n",
					baud);
			return false;
		}
		break;
	case 1800:
		cfsetispeed(&config, B1800);
		cfsetospeed(&config, B1800);
		break;
	case 9600:
		cfsetispeed(&config, B9600);
		cfsetospeed(&config, B9600);
		break;
	case 19200:
		cfsetispeed(&config, B19200);
		cfsetospeed(&config, B19200);
		break;
	case 38400:
		if (cfsetispeed(&config, B38400) < 0
				|| cfsetospeed(&config, B38400) < 0)
		{
			printf("ERROR: Could not set desired baud rate of %d Baud\n");
			return false;
		}
		break;
	case 57600:
		if (cfsetispeed(&config, B57600) < 0
				|| cfsetospeed(&config, B57600) < 0)
		{
			printf(	"ERROR: Could not set desired baud rate of %d Baud\n");
			return false;
		}
		break;
	case 115200:
		if (cfsetispeed(&config, B115200) < 0
				|| cfsetospeed(&config, B115200) < 0)
		{
			printf(	"ERROR: Could not set desired baud rate of %d Baud\n");
			return false;
		}
		break;

		// These two non-standard (by the 70'ties ) rates are fully supported on
		// current Debian and Mac OS versions (tested since 2010).
	case 460800:
		if (cfsetispeed(&config, B460800) < 0
				|| cfsetospeed(&config, B460800) < 0)
		{
			printf("ERROR: Could not set desired baud rate of %d Baud\n");
			return false;
		}
		break;
	case 921600:
		if (cfsetispeed(&config, B921600) < 0
				|| cfsetospeed(&config, B921600) < 0)
		{
			printf("ERROR: Could not set desired baud rate of %d Baud\n");
			return false;
		}
		break;
	default:
		printf("ERROR: Desired baud rate %d could not be set, aborting.\n");
		return false;
		break;
	}

	// Finally, apply the configuration
//	if (tcsetattr(m_fd, TCSANOW, &toptions) < 0)
//    fcntl(m_fd, F_SETFL, FNDELAY);
//	tcflush(m_fd, TCIOFLUSH);

	if (tcsetattr(m_fd, TCSAFLUSH, &config) < 0)
	{
		printf("ERROR: could not set configuration of fd %d\n");
		return false;
	}

	return true;
}


int SerialPort::Read(char *buffer, unsigned int nbChar)
{
	int n;

	// Lock
	pthread_mutex_lock(&m_portMutex);

	n = read(m_fd, buffer, nbChar);

	// Unlock
	pthread_mutex_unlock(&m_portMutex);

	return n;
}

void SerialPort::Write(char *buffer, unsigned int nbChar)
{
	// Lock
	pthread_mutex_lock(&m_portMutex);

	write(m_fd, buffer, nbChar);

	// Wait until all data has been written
	tcdrain(m_fd);

	// Unlock
	pthread_mutex_unlock(&m_portMutex);

}
#endif

