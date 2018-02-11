#include "../Script/Kiss.h"
#include "_SerialPort.h"

namespace kai
{

_SerialPort::_SerialPort(void)
{
	m_fd = -1;
	m_name = "";
	m_ioType = io_serialport;

	m_baud = 115200;
	m_dataBits = 8;
	m_stopBits = 1;
	m_parity = false;
	m_hardwareControl = false;
}

_SerialPort::~_SerialPort()
{
	reset();
}

bool _SerialPort::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("portName", &m_name));
	F_INFO(pK->v("baud", &m_baud));
	F_INFO(pK->v("dataBits", &m_dataBits));
	F_INFO(pK->v("stopBits", &m_stopBits));
	F_INFO(pK->v("parity", &m_parity));
	F_INFO(pK->v("hardwareControl", &m_hardwareControl));

	::sleep(1);
	open();

	return true;
}

bool _SerialPort::open(void)
{
	IF_F(m_name.empty());

//	m_fd = open(m_portName.c_str(), O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);	// | O_NDELAY);
	m_fd = ::open(m_name.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
	IF_F(m_fd == -1);
	fcntl(m_fd, F_SETFL, 0);

	m_ioStatus = io_opened;
	return setup();
}

void _SerialPort::close(void)
{
	IF_(m_ioStatus!=io_opened);

	::close(m_fd);
	this->_IOBase::close();
}

void _SerialPort::reset(void)
{
	this->_IOBase::reset();
	close();
}

bool _SerialPort::start(void)
{
	IF_T(m_bThreadON);

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		LOG_E(retCode);
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _SerialPort::update(void)
{
	while (m_bThreadON)
	{
		if (!isOpen())
		{
			if (!open())
			{
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		writeIO();
		readIO();

		if(!this->bEmptyW())
			this->disableSleep(true);
		else
			this->disableSleep(false);

		this->autoFPSto();
	}
}

void _SerialPort::writeIO(void)
{
	IO_BUF ioB;
	toBufW(&ioB);
	IF_(ioB.bEmpty());

	int nW = ::write(m_fd, ioB.m_pB, ioB.m_nB);

	// Wait until all data has been written
	tcdrain(m_fd);

	//if(nW!=nB)?

	LOG_I("Write: " << ioB.m_pB);
}

void _SerialPort::readIO(void)
{
	IO_BUF ioB;

	ioB.m_nB = ::read(m_fd, ioB.m_pB, N_IO_BUF);

	toQueR(&ioB);

	LOG_I("read: " << ioB.m_pB);
}

bool _SerialPort::setup(void)
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
			LOG(ERROR)<<"Could not set baud:"<<m_baud;
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
			LOG(ERROR)<<"Could not set baud:"<<m_baud;
			return false;
		}
		break;
	case 57600:
		if (cfsetispeed(&config, B57600) < 0
				|| cfsetospeed(&config, B57600) < 0)
		{
			LOG(ERROR)<<"Could not set baud:"<<m_baud;
			return false;
		}
		break;
	case 115200:
		if (cfsetispeed(&config, B115200) < 0
				|| cfsetospeed(&config, B115200) < 0)
		{
			LOG(ERROR)<<"Could not set baud:"<<m_baud;
			return false;
		}
		break;

		// These two non-standard (by the 70'ties ) rates are fully supported on
		// current Debian and Mac OS versions (tested since 2010).
	case 460800:
		if (cfsetispeed(&config, B460800) < 0
				|| cfsetospeed(&config, B460800) < 0)
		{
			LOG(ERROR)<<"Could not set baud:"<<m_baud;
			return false;
		}
		break;
	case 921600:
		if (cfsetispeed(&config, B921600) < 0
				|| cfsetospeed(&config, B921600) < 0)
		{
			LOG(ERROR)<<"Could not set baud:"<<m_baud;
			return false;
		}
		break;
	default:
		LOG(ERROR)<<"Could not set baud:"<<m_baud;
		return false;
		break;
	}

	//apply the configuration
//	if (tcsetattr(m_fd, TCSANOW, &toptions) < 0)
//  fcntl(m_fd, F_SETFL, FNDELAY);
//	tcflush(m_fd, TCIOFLUSH);

	if (tcsetattr(m_fd, TCSAFLUSH, &config) < 0)
	{
		LOG(ERROR)<<"Could not set configuration of fd:"<<m_fd;
		return false;
	}

	return true;
}

}

