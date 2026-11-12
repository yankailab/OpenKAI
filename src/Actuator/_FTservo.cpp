#include "_FTservo.h"
#include <cerrno>
#include <cmath>
#include <poll.h>

namespace kai
{

	void _FTservo::Serial::disconnect(void)
	{
		if (fd >= 0)
		{
			::close(fd);
			fd = -1;
		}
		txBufLen = 0;
	}

	int _FTservo::Serial::readSCS(unsigned char *pData, int nBytes)
	{
		return readSCS(pData, nBytes, IOTimeOut);
	}

	int _FTservo::Serial::readSCS(unsigned char *pData, int nBytes, unsigned long timeoutMs)
	{
		const uint64_t deadline = getTns() + timeoutMs * NSEC_MSEC;
		int nRead = 0;
		while (fd >= 0 && nRead < nBytes)
		{
			const uint64_t now = getTns();
			if (now >= deadline)
			{
				break;
			}
			pollfd descriptor = {fd, POLLIN, 0};
			const int remainingMs = static_cast<int>((deadline - now + NSEC_MSEC - 1) / NSEC_MSEC);
			const int ready = ::poll(&descriptor, 1, remainingMs);
			if (ready < 0 && errno == EINTR)
			{
				continue;
			}
			if (ready <= 0 || (descriptor.revents & (POLLERR | POLLHUP | POLLNVAL)))
			{
				break;
			}
			const int count = ::read(fd, pData + nRead, nBytes - nRead);
			if (count < 0 && (errno == EINTR || errno == EAGAIN))
			{
				continue;
			}
			if (count <= 0)
			{
				break;
			}
			nRead += count;
		}
		return nRead;
	}

	_FTservo::_FTservo()
	{
		m_ID = 1;
		m_mode = 0;
		m_p.m_vRange = Vector2f(0, 4095);
		m_p.m_vErr = 10;
		m_s.m_vRange = Vector2f(-32767, 32767);
		m_s.m_vTarget = 1000;
		m_a.m_vRange = Vector2f(0, 255);
		m_a.m_vTarget = 50;
	}

	_FTservo::~_FTservo()
	{
		stop();
	}

	bool _FTservo::loadConfig(void)
	{
		stop();
		m_bfSet.clearAll();
		m_bfStatus.clearAll();
		if (!_ActuatorBase::loadConfig())
		{
			return false;
		}
		const json &j = *m_pJ;
		jKv(j, "port", m_port);
		jKv(j, "baud", m_baud);
		if (m_port.empty() || m_ID < 0 || m_ID > 253 || (m_mode != 0 && m_mode != 1))
		{
			LOG_E("_FTservo requires port, ID in [0,253], and mode 0 (position) or 1 (wheel)");
			return false;
		}
		// SCSerial::begin silently falls back to 115200 for unsupported rates.
		if (m_baud != 9600 && m_baud != 19200 && m_baud != 38400 &&
			m_baud != 57600 && m_baud != 115200 && m_baud != 500000 && m_baud != 1000000)
		{
			LOG_E("Unsupported FTServo_Linux baud rate");
			return false;
		}
		for (ACTUATOR_V *pValue : {&m_p, &m_s, &m_a})
		{
			if (!pValue->m_vRange.allFinite() || pValue->m_vRange.x() > pValue->m_vRange.y() ||
				!std::isfinite(pValue->m_vTarget))
			{
				LOG_E("Invalid _FTservo command range or target");
				return false;
			}
			pValue->setTarget(pValue->m_vTarget);
		}
		if (m_bfSet.b(actuator_setID))
		{
			LOG_E("_FTservo ID selects an existing servo; ID programming is not supported");
			return false;
		}
		return true;
	}

	bool _FTservo::saveConfig(bool bExport)
	{
		if (!_ActuatorBase::saveConfig(false))
		{
			return false;
		}
		(*m_pJ)["port"] = m_port;
		(*m_pJ)["baud"] = m_baud;
		return !bExport || m_pJcfg->saveToFile();
	}

	bool _FTservo::start(void)
	{
		return m_pT && m_pT->startThread(getUpdate, this);
	}

	void _FTservo::stop(void)
	{
		// Shutdown must work even if the serial port has never opened.
		if (m_pT)
		{
			m_pT->join();
		}
		close();
	}

	bool _FTservo::check(void)
	{
		return m_bOpened && _ActuatorBase::check();
	}

	bool _FTservo::open(void)
	{
		if (m_bOpened)
		{
			return true;
		}
		if (!m_servo.begin(m_baud, m_port.c_str()))
		{
			m_servo.disconnect();
			return false;
		}
		m_bOpened = true;
		m_activeMode = -1;
		m_ieReadStatus.reset();
		m_ieSendCMD.reset();
		return true;
	}

	void _FTservo::close(void)
	{
		if (m_bOpened)
		{
			m_servo.EnableTorque(m_ID, 0);
		}
		m_servo.disconnect();
		m_bOpened = false;
		m_bTorque = false;
		m_bMoveSent = false;
		m_activeMode = -1;
		m_bfStatus.clear(actuator_powered);
		m_bfStatus.clear(actuator_ready);
		m_bfStatus.clear(actuator_moving);
		m_bfStatus.clear(actuator_complete);
	}

	bool _FTservo::commandOK(int result)
	{
		m_status = m_servo.getState();
		m_error = m_servo.getLastError();
		if (result <= 0 || m_status != 0 || m_error != 0)
		{
			m_bfStatus.set(actuator_alarm);
			m_bfStatus.clear(actuator_ready);
			return false;
		}
		return true;
	}

	bool _FTservo::readStatus(void)
	{
		if (!check() || !commandOK(m_servo.FeedBack(m_ID)))
		{
			return false;
		}
		// Decode only a complete successful packet, including signed values.
		m_p.set(m_servo.ReadPos(-1));
		m_s.set(m_servo.ReadSpeed(-1));
		m_c.set(m_servo.ReadCurrent(-1));
		m_bfStatus.clear(actuator_alarm);
		m_bfSet.clear(actuator_clearAlarm);
		if (m_activeMode >= 0)
		{
			m_bfStatus.set(actuator_ready);
		}
		m_bfStatus.clear(actuator_moving);
		if (m_servo.ReadMove(-1))
		{
			m_bfStatus.set(actuator_moving);
		}
		m_bfStatus.clear(actuator_complete);
		if (m_activeMode == 0 && m_p.bComplete())
		{
			m_bfStatus.set(actuator_complete);
		}
		return true;
	}

	bool _FTservo::stopMove(void)
	{
		if (m_activeMode == 1)
		{
			return commandOK(m_servo.WriteSpe(m_ID, 0, 0));
		}
		if (!readStatus())
		{
			return false;
		}
		return commandOK(m_servo.WritePosEx(m_ID, static_cast<s16>(m_p.get()), 0, 0));
	}

	bool _FTservo::updateMove(void)
	{
		if (!check() || (m_mode != 0 && m_mode != 1))
		{
			return false;
		}
		if (m_activeMode != m_mode || m_bfSet.b(actuator_setMode))
		{
			if (!commandOK(m_servo.EnableTorque(m_ID, 0)))
			{
				return false;
			}
			m_bTorque = false;
			m_bMoveSent = false;
			m_bfStatus.clear(actuator_powered);
			const int result = m_mode == 0 ? m_servo.ServoMode(m_ID) : m_servo.WheelMode(m_ID);
			if (!commandOK(result))
			{
				return false;
			}
			m_activeMode = m_mode;
			m_bfSet.clear(actuator_setMode);
		}

		const bool bPower = m_bfSet.b(actuator_power);
		if (bPower != m_bTorque)
		{
			// Replace any old hardware goal before torque is enabled.
			if (bPower && !stopMove())
			{
				return false;
			}
			if (!commandOK(m_servo.EnableTorque(m_ID, bPower ? 1 : 0)))
			{
				return false;
			}
			m_bTorque = bPower;
			m_bMoveSent = false;
			m_bfStatus.clear(actuator_powered);
			if (bPower)
			{
				m_bfStatus.set(actuator_powered);
			}
		}
		if (!bPower)
		{
			return true;
		}
		const bool bMove = m_bfSet.b(actuator_move) || (m_mode == 0 && m_bfSet.b(actuator_gotoOrigin));
		if (!bMove)
		{
			if (m_bMoveSent && !stopMove())
			{
				return false;
			}
			m_bMoveSent = false;
			return true;
		}

		for (ACTUATOR_V *pValue : {&m_p, &m_s, &m_a})
		{
			if (!std::isfinite(pValue->getTarget()))
			{
				return false;
			}
			pValue->setTarget(pValue->getTarget());
		}
		const s16 position = static_cast<s16>(std::clamp(m_p.getTarget(), -32767.0f, 32767.0f));
		const s16 speed = static_cast<s16>(std::clamp(m_s.getTarget(), -32767.0f, 32767.0f));
		const u8 accel = static_cast<u8>(std::clamp(m_a.getTarget(), 0.0f, 255.0f));
		const int result = m_mode == 0
			? m_servo.WritePosEx(m_ID, position, static_cast<u16>(std::abs(speed)), accel)
			: m_servo.WriteSpe(m_ID, speed, accel);
		if (!commandOK(result))
		{
			return false;
		}
		m_bMoveSent = true;
		setLastCmdTime();
		// Consume the one-shot origin request into normal movement, so a later
		// move(false) can stop it using the shared actuator flag.
		if (m_mode == 0 && m_bfSet.b(actuator_gotoOrigin, true))
		{
			m_bfSet.set(actuator_move);
		}
		return true;
	}

	void _FTservo::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();
			if (!m_pT->bRun())
			{
				break;
			}
			if (!open())
			{
				LOG_E("Cannot open _FTservo port: " + m_port);
				m_pT->sleepT(NSEC_SEC);
				continue;
			}
			const uint64_t now = getTns();
			if ((m_ieReadStatus.update(now) && !readStatus()) ||
				(m_ieSendCMD.update(now) && !updateMove()))
			{
				LOG_E("_FTservo communication/command failed, status=" + i2str(m_status) + ", error=" + i2str(m_error));
				m_bfStatus.set(actuator_alarm);
				close();
				m_pT->sleepT(NSEC_SEC);
			}
		}
		close();
	}

	void _FTservo::console(void *pConsole)
	{
		if (!pConsole)
		{
			return;
		}
		_ActuatorBase::console(pConsole);
		_Console *pC = static_cast<_Console *>(pConsole);
		pC->addMsg("_FTservo " + m_port + " @ " + i2str(m_baud) +
			", opened=" + i2str(m_bOpened) + ", mode=" + i2str(m_activeMode) +
			", torque=" + i2str(m_bTorque) + ", status=" + i2str(m_status) + ", error=" + i2str(m_error), 1);
	}

}
