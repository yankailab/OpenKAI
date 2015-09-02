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
        DCB dcb = {0};

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
	m_fd = 0;
}

bool SerialPort::IsConnected()
{
	//Simply return the connection status
	return this->bConnected;
}

SerialPort::~SerialPort()
{
    //Check if we are bConnected before trying to disconnect
    if(this->bConnected)
    {
        //We're no longer bConnected
        this->bConnected = false;
        //Close the serial file descriptor
        close(m_fd);
    }
}

bool SerialPort::connect(char *portName)
{
    //We're not yet bConnected
    this->bConnected = false;

    struct termios toptions;

//    char* nametest = "/dev/ttyACM0";
    //fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
    m_fd = open(/*portName*/"/dev/cu.usbmodem1411", O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);// | O_NDELAY);// | O_NONBLOCK);
    if(m_fd == -1)
    {
	  printf("init_serialport: Unable to open port\n");
	  return false;
    }

    if(tcgetattr(m_fd, &toptions) < 0)
    {
	  printf("init_serialport: Couldn't get term attributes\n");
	  return false;
    }

    speed_t brate = B9600;//B115200; // let you override switch below if needed
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;

    // no flow control
    toptions.c_cflag &= ~CRTSCTS;
//    toptions.c_cflag |= CRTSCTS;

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY);// | BRKINT | ICRNL | INPCK | ISTRIP | IXON | IUCLC | INLCR| IXANY); // turn off s/w flow ctrl
    toptions.c_iflag |= IGNPAR;
<<<<<<< Updated upstream

=======
>>>>>>> Stashed changes
    toptions.c_lflag = 0;
    toptions.c_oflag = 0;
//    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN); // make raw
//    toptions.c_oflag &= ~(OPOST|OLCUC|ONLCR|OCRNL|ONLRET|OFDEL); // make raw

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 10;

    if(tcsetattr(m_fd, TCSANOW, &toptions) < 0)
    {
	  printf("init_serialport: Couldn't set term attributes\n");
	  return false;
    }

//    fcntl(m_fd, F_SETFL, FNDELAY);
    tcflush(m_fd,TCIOFLUSH);
/*
    struct serial_struct serial;
    ioctl(m_fd, TIOCGSERIAL, &serial);
    serial.flags |= ASYNC_LOW_LATENCY; // (0x2000)
//    serial.xmit_fifo_size = 8;
    ioctl(m_fd, TIOCSSERIAL, &serial);


	usleep(2000 * 1000);
	this->bConnected = true;

    return true;
}

int SerialPort::ReadData(char *buffer, unsigned int nbChar)
{
	int n = read(m_fd, buffer, nbChar);  // read a char at a time
	if(n<=0)
	{
		return -1;    // couldn't read or nothing to read
	}

<<<<<<< Updated upstream

//    tcflush(m_fd,TCIFLUSH);
=======
	tcflush(m_fd,TCIFLUSH);

>>>>>>> Stashed changes
    return n;
}


bool SerialPort::WriteData(char *buffer, unsigned int nbChar)
{
    unsigned int n = write(m_fd, buffer, nbChar);

    if( n!=nbChar )
    {
//    	printf("Not written! %d, %d\n",nbChar, n);
    		return false;
    }

    return true;
}
#endif





