/*
 * _PCrecv.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#include "_PCrecv.h"

namespace kai
{

	_PCrecv::_PCrecv()
	{
		m_pIO = nullptr;
		m_nCMDrecv = 0;

		m_nRead = 0;
		m_iRead = 0;
	}

	_PCrecv::~_PCrecv()
	{
	}

	bool _PCrecv::init(const json &j)
	{
		IF_F(!this->_PCstream::init(j));

		return true;
	}

	bool _PCrecv::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_PCstream::link(j, pM));

		string n = j.value("_IObase", "");
		m_pIO = (_IObase *)(pM->findModule(n));
		NULL_F(m_pIO);

		return true;
	}

	bool _PCrecv::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	bool _PCrecv::check(void)
	{
		NULL_F(m_pIO);
		IF_F(!m_pIO->bOpen());

		return this->_PCstream::check();
	}

	void _PCrecv::update(void)
	{
		PROTOCOL_CMD rCMD;

		while (m_pT->bAlive())
		{
			IF_CONT(!readCMD(&rCMD));

			handleCMD(rCMD);
			rCMD.clear();
			m_nCMDrecv++;
		}
	}

	bool _PCrecv::readCMD(PROTOCOL_CMD *pCmd)
	{
		IF_F(!check());
		NULL_F(pCmd);

		if (m_nRead == 0)
		{
			m_nRead = m_pIO->read(m_pBuf, PB_N_BUF);
			IF_F(m_nRead <= 0);
			m_iRead = 0;
		}

		while (m_iRead < m_nRead)
		{
			bool r = pCmd->input(m_pBuf[m_iRead++]);
			if (m_iRead == m_nRead)
			{
				m_iRead = 0;
				m_nRead = 0;
			}

			IF__(r, true);
		}

		return false;
	}

	void _PCrecv::handleCMD(const PROTOCOL_CMD &cmd)
	{
		switch (cmd.m_pB[1])
		{
		case PC_STREAM:
		{
			decodeStream(cmd);
			break;
		}
		case PC_FRAME_END:
		{
			break;
		}
		default:
			break;
		}
	}

	void _PCrecv::decodeStream(const PROTOCOL_CMD &cmd)
	{
		const double PC_SCALE_INV = 0.001;
		int16_t x, y, z;

		for (int i = PC_N_HDR; i < cmd.m_nPayload + PC_N_HDR; i += 12)
		{
			IF_(i + 12 > PC_N_BUF);

			x = unpack_int16(&cmd.m_pB[i], false);
			y = unpack_int16(&cmd.m_pB[i + 2], false);
			z = unpack_int16(&cmd.m_pB[i + 4], false);
			Eigen::Vector3d vP(((double)x) * PC_SCALE_INV,
							   ((double)y) * PC_SCALE_INV,
							   ((double)z) * PC_SCALE_INV);

			x = unpack_int16(&cmd.m_pB[i + 6], false);
			y = unpack_int16(&cmd.m_pB[i + 8], false);
			z = unpack_int16(&cmd.m_pB[i + 10], false);
			Eigen::Vector3f vC(((float)x) * PC_SCALE_INV,
							   ((float)y) * PC_SCALE_INV,
							   ((float)z) * PC_SCALE_INV);

			add(vP, vC, m_pT->getTfrom());
		}
	}

}
