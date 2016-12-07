#include "SerialPort.h"
#include "../Script/Kiss.h"

namespace kai
{

SerialPort::SerialPort(void)
{
	IO();
	m_fd = -1;
	m_name = "";
	m_type = serialport;

	m_baud = 115200;
	m_dataBits = 8;
	m_stopBits = 1;
	m_parity = false;
	m_hardwareControl = false;


	pthread_mutex_init(&m_mutexWrite, NULL);
	pthread_mutex_init(&m_mutexRead, NULL);
}

SerialPort::~SerialPort()
{
	close();

	pthread_mutex_destroy(&m_mutexWrite);
	pthread_mutex_destroy(&m_mutexRead);
}

bool SerialPort::init(void* pKiss)
{
	CHECK_F(!this->IO::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("portName", &m_name));
	F_INFO(pK->v("baud", &m_baud));
	F_INFO(pK->v("dataBits", &m_dataBits));
	F_INFO(pK->v("stopBits", &m_stopBits));
	F_INFO(pK->v("parity", &m_parity));
	F_INFO(pK->v("hardwareControl", &m_hardwareControl));

	return true;
}

bool SerialPort::open(void)
{
	CHECK_F(m_name.empty());

//	m_fd = open(m_portName.c_str(), O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);	// | O_NDELAY);
	m_fd = ::open(m_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
	CHECK_F(m_fd == -1);
	fcntl(m_fd, F_SETFL, 0);

	m_status = opening;
	return setup();
}

void SerialPort::close(void)
{
	CHECK_(m_status!=opening);

	::close(m_fd);
	m_status = closed;
}

int SerialPort::read(uint8_t* pBuf, int nByte)
{
	int n;

	pthread_mutex_lock(&m_mutexRead);
	n = ::read(m_fd, pBuf, nByte);
	pthread_mutex_unlock(&m_mutexRead);

	return n;
}

bool SerialPort::write(uint8_t* pBuf, int nByte)
{
	int n;

	pthread_mutex_lock(&m_mutexWrite);
	n = ::write(m_fd, pBuf, nByte);
	// Wait until all data has been written
	tcdrain(m_fd);
	pthread_mutex_unlock(&m_mutexWrite);

	return (n==nByte);
}

bool SerialPort::writeLine(uint8_t* pBuf, int nByte)
{
	int n;
	const char crlf[] = "\x0d\x0a";

	pthread_mutex_lock(&m_mutexWrite);
	n = ::write(m_fd, pBuf, nByte);
	n += ::write(m_fd, crlf, 2);
	// Wait until all data has been written
	tcdrain(m_fd);
	pthread_mutex_unlock(&m_mutexWrite);

	return (n==nByte+2);
}

bool SerialPort::setup(void)
{
	// Check file descriptor
	if (!isatty(m_fd))
	{
		printf("ERROR: file descriptor is NOT a serial port\n");
		return false;
	}

	// Read file descritor configuration
	struct termios config;
	if (tcgetattr(m_fd, &config) < 0)
	{
		printf("\nERROR: could not read configuration of fd\n");
		return false;
	}

	// Input flags - Turn off input processing
	// convert break to null byte, no CR to NL translation,
	// no NL to CR translation, don't mark parity errors or breaks
	// no input parity check, don't strip high bit off,
	// no XON/XOFF software flow control
	config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK
			| ISTRIP | IXON);
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
	config.c_cc[VTIME] = 0; // was 0
	// see: http://unixwiz.net/techtips/termios-vmin-vtime.html
//	toptions.c_cc[VMIN] = 0;
//	toptions.c_cc[VTIME] = 10;

	// Apply baudrate
	switch (m_baud)
	{
	case 1200:
		if (cfsetispeed(&config, B1200) < 0 || cfsetospeed(&config, B1200) < 0)
		{
			fprintf(stderr,
					"\nERROR: Could not set desired baud rate of %d Baud\n",
					m_baud);
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
			printf("ERROR: Could not set desired baud rate of %d Baud\n", m_baud);
			return false;
		}
		break;
	case 57600:
		if (cfsetispeed(&config, B57600) < 0
				|| cfsetospeed(&config, B57600) < 0)
		{
			printf("ERROR: Could not set desired baud rate of %d Baud\n", m_baud);
			return false;
		}
		break;
	case 115200:
		if (cfsetispeed(&config, B115200) < 0
				|| cfsetospeed(&config, B115200) < 0)
		{
			printf("ERROR: Could not set desired baud rate of %d Baud\n", m_baud);
			return false;
		}
		break;

		// These two non-standard (by the 70'ties ) rates are fully supported on
		// current Debian and Mac OS versions (tested since 2010).
	case 460800:
		if (cfsetispeed(&config, B460800) < 0
				|| cfsetospeed(&config, B460800) < 0)
		{
			printf("ERROR: Could not set desired baud rate of %d Baud\n", m_baud);
			return false;
		}
		break;
	case 921600:
		if (cfsetispeed(&config, B921600) < 0
				|| cfsetospeed(&config, B921600) < 0)
		{
			printf("ERROR: Could not set desired baud rate of %d Baud\n", m_baud);
			return false;
		}
		break;
	default:
		printf("ERROR: Desired baud rate %d could not be set, aborting.\n",
				m_baud);
		return false;
		break;
	}

	//apply the configuration
//	if (tcsetattr(m_fd, TCSANOW, &toptions) < 0)
//  fcntl(m_fd, F_SETFL, FNDELAY);
//	tcflush(m_fd, TCIOFLUSH);

	if (tcsetattr(m_fd, TCSAFLUSH, &config) < 0)
	{
		printf("ERROR: could not set configuration of fd %d\n", m_fd);
		return false;
	}

	return true;
}

}

