/*
 * _PCrecv.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#include "_PCrecv.h"

#ifdef USE_OPEN3D

namespace kai
{

_PCrecv::_PCrecv()
{
	m_pIO = NULL;
    m_nCMDrecv = 0;
}

_PCrecv::~_PCrecv()
{
}

bool _PCrecv::init(void *pKiss)
{
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

    int nB = 256;
    pK->v("nB", &nB);
	m_recvMsg.init(nB);

	string n;
	n = "";
	F_ERROR_F(pK->v("_IOBase", &n));
	m_pIO = (_IOBase*) (pK->getInst(n));
	NULL_Fl(m_pIO, "_IOBase not found");
    
	return true;
}

bool _PCrecv::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

int _PCrecv::check(void)
{
	NULL__(m_pIO, -1);
	IF__(!m_pIO->isOpen(),-1);

	return this->_PCbase::check();
}

void _PCrecv::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		while(readCMD())
		{
			handleCMD();
			m_nCMDrecv++;
		}

		m_pT->autoFPSto();
	}
}

bool _PCrecv::readCMD(void)
{
    IF_F(check()<0);
    
	uint8_t	b;
	while (m_pIO->read(&b,1) > 0)
	{
		if (m_recvMsg.m_cmd != 0)
		{
			m_recvMsg.m_pB[m_recvMsg.m_iB] = b;
			m_recvMsg.m_iB++;

			if (m_recvMsg.m_iB == 4)
			{
				m_recvMsg.m_nPayload = unpack_int16(&m_recvMsg.m_pB[2], false);
			}
			
			IF_T(m_recvMsg.m_iB >= m_recvMsg.m_nPayload + PC_N_HDR );
            IF_T(m_recvMsg.m_iB >= m_recvMsg.m_nB);
		}
		else if (b == PB_BEGIN )
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
	PointCloud* pPC = m_sPC.next();
        
	switch (m_recvMsg.m_pB[1])
	{
	case PC_STREAM:
    {
        decodeStream();
		break;
    }
	case PC_FRAME_END:
    {
        updatePC();
        
   		if(m_pViewer)
		{
			m_pViewer->updateGeometry(m_iV, m_sPC.prev());
		}

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
   	int16_t x,y,z;
	PointCloud* pPC = m_sPC.next();

    for(int i=PC_N_HDR; i<m_recvMsg.m_nPayload + PC_N_HDR; i+=12)
    {
        IF_(i + 12 > m_recvMsg.m_nB);
        
        x = unpack_int16(&m_recvMsg.m_pB[i], false);
        y = unpack_int16(&m_recvMsg.m_pB[i+2], false);
        z = unpack_int16(&m_recvMsg.m_pB[i+4], false);
        Eigen::Vector3d vp(((double)x)*PC_SCALE_INV,
                            ((double)y)*PC_SCALE_INV,
                            ((double)z)*PC_SCALE_INV);
        pPC->points_.push_back(vp);

        x = unpack_int16(&m_recvMsg.m_pB[i+6], false);
        y = unpack_int16(&m_recvMsg.m_pB[i+8], false);
        z = unpack_int16(&m_recvMsg.m_pB[i+10], false);
        Eigen::Vector3d vc(((double)x)*PC_SCALE_INV,
                            ((double)y)*PC_SCALE_INV,
                            ((double)z)*PC_SCALE_INV);
        pPC->colors_.push_back(vc);
    }
}

}
#endif
