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
	}

	_PCrecv::~_PCrecv()
	{
	}

	int _PCrecv::init(void *pKiss)
	{
		CHECK_(_PCstream::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		int nB = 256;
		pK->v("nB", &nB);
		m_recvMsg.init(nB);

        string n;
        n = "";
        if(!pK->v("_IObase", &n))
        {
            LOG_E("_IObase not found");
            return OK_ERR_NOT_FOUND;
        }

        m_pIO = (_IObase *)(pK->findModule(n));
        if(!m_pIO)
        {
            LOG_E("_IObase not found");
            return OK_ERR_NOT_FOUND;
        }

		return true;
	}

	int _PCrecv::start(void)
	{
		NULL__(m_pT, OK_ERR_NULLPTR);
		return m_pT->start(getUpdate, this);
	}

	int _PCrecv::check(void)
	{
		NULL__(m_pIO, OK_ERR_NULLPTR);
		IF__(!m_pIO->bOpen(), OK_ERR_NOT_READY);

		return this->_PCstream::check();
	}

	void _PCrecv::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPSfrom();

			while (readCMD())
			{
				handleCMD();
				m_nCMDrecv++;
			}

			m_pT->autoFPSto();
		}
	}

	bool _PCrecv::readCMD(void)
	{
		IF_F(check() < 0);

		uint8_t b;
		while (m_pIO->read(&b, 1) > 0)
		{
			if (m_recvMsg.m_cmd != 0)
			{
				m_recvMsg.m_pB[m_recvMsg.m_iB] = b;
				m_recvMsg.m_iB++;

				if (m_recvMsg.m_iB == 4)
				{
					m_recvMsg.m_nPayload = unpack_int16(&m_recvMsg.m_pB[2], false);
				}

				IF_T(m_recvMsg.m_iB >= m_recvMsg.m_nPayload + PC_N_HDR);
				IF_T(m_recvMsg.m_iB >= m_recvMsg.m_nB);
			}
			else if (b == PB_BEGIN)
			{
				m_recvMsg.m_cmd = b;
				m_recvMsg.m_pB[0] = b;
				m_recvMsg.m_iB = 1;
				m_recvMsg.m_nPayload = 0;
			}
		}

		return false;
	}

	void _PCrecv::handleCMD(void)
	{
		switch (m_recvMsg.m_pB[1])
		{
		case PC_STREAM:
		{
			decodeStream();
			break;
		}
		case PC_FRAME_END:
		{
			break;
		}
		default:
			break;
		}

		m_recvMsg.reset();
	}

	void _PCrecv::decodeStream(void)
	{
		const double PC_SCALE_INV = 0.001;
		int16_t x, y, z;

		for (int i = PC_N_HDR; i < m_recvMsg.m_nPayload + PC_N_HDR; i += 12)
		{
			IF_(i + 12 > m_recvMsg.m_nB);

			x = unpack_int16(&m_recvMsg.m_pB[i], false);
			y = unpack_int16(&m_recvMsg.m_pB[i + 2], false);
			z = unpack_int16(&m_recvMsg.m_pB[i + 4], false);
			Eigen::Vector3d vP(((double)x) * PC_SCALE_INV,
							   ((double)y) * PC_SCALE_INV,
							   ((double)z) * PC_SCALE_INV);

			x = unpack_int16(&m_recvMsg.m_pB[i + 6], false);
			y = unpack_int16(&m_recvMsg.m_pB[i + 8], false);
			z = unpack_int16(&m_recvMsg.m_pB[i + 10], false);
			Eigen::Vector3f vC(((float)x) * PC_SCALE_INV,
							   ((float)y) * PC_SCALE_INV,
							   ((float)z) * PC_SCALE_INV);

			add(vP, vC, m_pT->getTfrom());
		}
	}

}
