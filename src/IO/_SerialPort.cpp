#include "../Script/Kiss.h"
#include "_SerialPort.h"

namespace kai
{

_SerialPort::_SerialPort(void)
{
	m_fd = -1;
	m_port = "";
	m_ioType = io_serialPort;

	m_baud = 115200;
	m_dataBits = 8;
	m_stopBits = 1;
	m_parity = false;
	m_hardwareControl = false;
}

_SerialPort::~_SerialPort()
{
	close();
}

bool _SerialPort::init(void* pKiss)
{
	IF_F(!this->_IOBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("port",&m_port);
	pK->v("baud",&m_baud);
	pK->v("dataBits",&m_dataBits);
	pK->v("stopBits",&m_stopBits);
	pK->v("parity",&m_parity);
	pK->v("hardwareControl",&m_hardwareControl);

	return true;
}

bool _SerialPort::open(void)
{
	IF_Fl(m_port.empty(), "port is empty");

	m_fd = ::open(m_port.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);	//O_SYNC | O_NONBLOCK);
	IF_Fl(m_fd == -1, "Cannot open: " + m_port);
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

bool _SerialPort::start(void)
{
	int retCode;

	if(!m_bThreadON)
	{
		m_bThreadON = true;
		retCode = pthread_create(&m_threadID, 0, getUpdateThreadW, this);
		if (retCode != 0)
		{
			LOG_E(retCode);
			m_bThreadON = false;
			return false;
		}
	}

	if(!m_bRThreadON)
	{
		m_bRThreadON = true;
		retCode = pthread_create(&m_rThreadID, 0, getUpdateThreadR, this);
		if (retCode != 0)
		{
			LOG_E(retCode);
			m_bRThreadON = false;
			return false;
		}
	}

	return true;
}

void _SerialPort::updateW(void)
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

		uint8_t pB[N_IO_BUF];
		int nB;
		while((nB = m_fifoW.output(pB, N_IO_BUF)) > 0)
		{
			int nW = ::write(m_fd, pB, nB);
			LOG_I("write: " + i2str(nW) + " bytes");
		}

		tcdrain(m_fd);

		this->autoFPSto();
	}
}

void _SerialPort::updateR(void)
{
	while (m_bRThreadON)
	{
		if (!isOpen())
		{
			::sleep(1);
			continue;
		}

		uint8_t pB[N_IO_BUF];
		int nR = ::read(m_fd, pB, N_IO_BUF);
		IF_CONT(nR <= 0);

		m_fifoR.input(pB,nR);
		this->wakeUpLinked();

		LOG_I("read: " + i2str(nR) + " bytes");
	}
}

bool _SerialPort::setup(void)
{
	// Check file descriptor
	IF_Fl(!isatty(m_fd), "file descriptor is NOT a serial port");

	// Read file descritor configuration
	struct termios config;
	IF_Fl(tcgetattr(m_fd, &config) < 0, "could not read configuration of fd");

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
	config.c_cc[VTIME] = 0;
	// see: http://unixwiz.net/techtips/termios-vmin-vtime.html
//	toptions.c_cc[VMIN] = 0;
//	toptions.c_cc[VTIME] = 10;

	// Apply baudrate
	switch (m_baud)
	{
	case 1200:
		if (cfsetispeed(&config, B1200) < 0 || cfsetospeed(&config, B1200) < 0)
		{
			LOG_E("Could not set baud: " + i2str(m_baud));
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
			LOG_E("Could not set baud: " + i2str(m_baud));
			return false;
		}
		break;
	case 57600:
		if (cfsetispeed(&config, B57600) < 0
				|| cfsetospeed(&config, B57600) < 0)
		{
			LOG_E("Could not set baud: " + i2str(m_baud));
			return false;
		}
		break;
	case 115200:
		if (cfsetispeed(&config, B115200) < 0
				|| cfsetospeed(&config, B115200) < 0)
		{
			LOG_E("Could not set baud: " + i2str(m_baud));
			return false;
		}
		break;
	case 230400:
		if (cfsetispeed(&config, B230400) < 0
				|| cfsetospeed(&config, B230400) < 0)
		{
			LOG_E("Could not set baud: " + i2str(m_baud));
			return false;
		}
		break;

		// These two non-standard (by the 70'ties ) rates are fully supported on
		// current Debian and Mac OS versions (tested since 2010).
	case 460800:
		if (cfsetispeed(&config, B460800) < 0
				|| cfsetospeed(&config, B460800) < 0)
		{
			LOG_E("Could not set baud: " + i2str(m_baud));
			return false;
		}
		break;
	case 921600:
		if (cfsetispeed(&config, B921600) < 0
				|| cfsetospeed(&config, B921600) < 0)
		{
			LOG_E("Could not set baud: " + i2str(m_baud));
			return false;
		}
		break;
	default:
		LOG_E("Could not set baud: " + i2str(m_baud));
		return false;
		break;
	}

	//apply the configuration
	if (tcsetattr(m_fd, TCSAFLUSH, &config) < 0)
	{
		LOG_E("Could not set configuration of fd: " + i2str(m_fd));
		return false;
	}

	return true;
}

}

