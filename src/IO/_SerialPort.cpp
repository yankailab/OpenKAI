#include "_SerialPort.h"

namespace kai
{

	_SerialPort::_SerialPort(void)
	{
	}

	_SerialPort::~_SerialPort()
	{
		stop();
		close();
	}

	bool _SerialPort::loadConfig(void)
	{
		IF_F(!this->_IObase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "port", m_port);
		jKv(j, "baud", m_baud);
		jKv(j, "dataBits", m_dataBits);
		jKv(j, "stopBits", m_stopBits);
		jKv(j, "parity", m_parity);
		jKv(j, "hardwareControl", m_hardwareControl);

		return true;
	}

	bool _SerialPort::saveConfig(bool bExport)
	{
		IF_F(!_IObase::saveConfig(false));

		json &j = *m_pJ;
		j["port"] = m_port;
		j["baud"] = m_baud;
		j["dataBits"] = m_dataBits;
		j["stopBits"] = m_stopBits;
		j["parity"] = m_parity;
		j["hardwareControl"] = m_hardwareControl;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _SerialPort::link(InstanceMgr *pM)
	{
		IF_F(!this->_IObase::link(pM));

		return true;
	}

	bool _SerialPort::open(void)
	{
		std::unique_lock<std::shared_mutex> lock(m_connectionMutex);
		if (bOpen())
		{
			return true;
		}
		if (m_port.empty())
		{
			LOG_E("port is empty");
			return false;
		}

		m_fd = ::open(m_port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
		if (m_fd < 0)
		{
			LOG_E("Cannot open: " + m_port);
			return false;
		}
		if (!setup())
		{
			::close(m_fd);
			m_fd = -1;
			return false;
		}

		++m_connectionGeneration;
		m_ioStatus = io_opened;
		return true;
	}

	void _SerialPort::close(void)
	{
		closeConnection();
	}

	void _SerialPort::closeConnection(uint64_t generation)
	{
		std::unique_lock<std::shared_mutex> lock(m_connectionMutex);
		if (generation != 0 && generation != m_connectionGeneration)
		{
			return;
		}
		if (m_fd >= 0)
		{
			::close(m_fd);
			m_fd = -1;
		}
		_IObase::close();
	}

	void _SerialPort::readPackets(void)
	{
		if (!m_pBpStreamOut)
		{
			return;
		}

		uint8_t pB[N_SERIAL_BUF];
		while (bOpen())
		{
			if (m_pTr && !m_pTr->bRun())
			{
				return;
			}

			ssize_t nR;
			int error;
			uint64_t generation;
			{
				std::shared_lock<std::shared_mutex> lock(m_connectionMutex);
				if (!bOpen() || m_fd < 0)
				{
					return;
				}
				generation = m_connectionGeneration;
				nR = ::read(m_fd, pB, sizeof(pB));
				error = errno;
			}
			if (nR > 0)
			{
				m_pBpStreamOut->addPacket(vector<uint8_t>(pB, pB + nR));
				continue;
			}
			if (nR < 0 && error == EINTR)
			{
				continue;
			}
			if (nR < 0 && error != EAGAIN && error != EWOULDBLOCK)
			{
				LOG_E("read error: " + i2str(error));
				closeConnection(generation);
			}
			return;
		}
	}

	void _SerialPort::writePackets(void)
	{
		uint64_t generation = m_connectionGeneration;
		if (m_writeConnectionGeneration != generation)
		{
			// A reconnect restarts the pending packet; only the writer owns its offset.
			m_iWrite = 0;
			m_writeConnectionGeneration = generation;
		}
		if (!writePending() || !m_pBpStreamIn)
		{
			return;
		}

		vector<BYTE_PACKET> vBp;
		m_pBpStreamIn->getPackets(vBp, m_tLastBpStreamIn);
		for (const BYTE_PACKET &bp : vBp)
		{
			m_bpWrite = bp;
			if (!writePending())
			{
				break;
			}
		}
	}

	bool _SerialPort::writePending(void)
	{
		while (m_iWrite < m_bpWrite.m_vB.size())
		{
			if (m_pT && !m_pT->bRun())
			{
				return false;
			}

			ssize_t nW;
			int error;
			{
				std::shared_lock<std::shared_mutex> lock(m_connectionMutex);
				if (!bOpen() || m_fd < 0 || m_writeConnectionGeneration != m_connectionGeneration)
				{
					return false;
				}
				nW = ::write(m_fd, m_bpWrite.m_vB.data() + m_iWrite,
							  m_bpWrite.m_vB.size() - m_iWrite);
				error = errno;
			}
			if (nW < 0 && error == EINTR)
			{
				continue;
			}
			if (nW < 0 && error != EAGAIN && error != EWOULDBLOCK)
			{
				LOG_E("write error: " + i2str(error));
				closeConnection(m_writeConnectionGeneration);
				return false;
			}
			if (nW <= 0)
			{
				return false;
			}

			m_iWrite += static_cast<size_t>(nW);
			LOG_I("write: " + i2str(nW) + " bytes");
		}

		if (m_bpWrite.m_tStamp > m_tLastBpStreamIn)
		{
			m_tLastBpStreamIn = m_bpWrite.m_tStamp;
		}
		m_bpWrite.clear();
		m_iWrite = 0;
		return true;
	}

	bool _SerialPort::setup(void)
	{
		// Check file descriptor
		if (!isatty(m_fd))
		{
			LOG_E("file descriptor is NOT a serial port");
			return false;
		}

		// Read file descritor configuration
		struct termios config;
		if (tcgetattr(m_fd, &config) < 0)
		{
			LOG_E("could not read configuration of fd");
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
			if (cfsetispeed(&config, B38400) < 0 || cfsetospeed(&config, B38400) < 0)
			{
				LOG_E("Could not set baud: " + i2str(m_baud));
				return false;
			}
			break;
		case 57600:
			if (cfsetispeed(&config, B57600) < 0 || cfsetospeed(&config, B57600) < 0)
			{
				LOG_E("Could not set baud: " + i2str(m_baud));
				return false;
			}
			break;
		case 115200:
			if (cfsetispeed(&config, B115200) < 0 || cfsetospeed(&config, B115200) < 0)
			{
				LOG_E("Could not set baud: " + i2str(m_baud));
				return false;
			}
			break;
		case 230400:
			if (cfsetispeed(&config, B230400) < 0 || cfsetospeed(&config, B230400) < 0)
			{
				LOG_E("Could not set baud: " + i2str(m_baud));
				return false;
			}
			break;

			// These two non-standard (by the 70'ties ) rates are fully supported on
			// current Debian and Mac OS versions (tested since 2010).
		case 460800:
			if (cfsetispeed(&config, B460800) < 0 || cfsetospeed(&config, B460800) < 0)
			{
				LOG_E("Could not set baud: " + i2str(m_baud));
				return false;
			}
			break;
		case 921600:
			if (cfsetispeed(&config, B921600) < 0 || cfsetospeed(&config, B921600) < 0)
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

		// apply the configuration
		if (tcsetattr(m_fd, TCSAFLUSH, &config) < 0)
		{
			LOG_E("Could not set configuration of fd: " + i2str(m_fd));
			return false;
		}

		return true;
	}

	void _SerialPort::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_IObase::console(pConsole);
	}

}
