/*
 * _PCio.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: yankai
 */

#include "_PCsend.h"

#ifdef USE_OPEN3D

namespace kai
{

_PCsend::_PCsend()
{
	m_pIO = NULL;
	m_pB = NULL;
	m_nB = 1024;
}

_PCsend::~_PCsend()
{
}

bool _PCsend::init(void *pKiss)
{
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("nB", &m_nB);
	m_pB = new uint8_t[m_nB];
    NULL_F(m_pB);

	string n;
	n = "";
	F_ERROR_F(pK->v("_IOBase", &n));
	m_pIO = (_IOBase*) (pK->getInst(n));
	NULL_Fl(m_pIO, "_IOBase not found");

	return true;
}

bool _PCsend::start(void)
{
	IF_F(!this->_ThreadBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

int _PCsend::check(void)
{
	NULL__(m_pPCB, -1);
	NULL__(m_pIO, -1);
	IF__(!m_pIO->isOpen(),-1);

	return 0;
}

void _PCsend::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateIO();
		m_sPC.update();

		this->autoFPSto();
	}
}

void _PCsend::updateIO(void)
{
	IF_(check()<0);

	*m_sPC.next() = *m_pPCB->getPC();
	PointCloud* pOut = m_sPC.next();
	int nP = pOut->points_.size();

    const double PC_SCALE = 1000;
    int iB = PB_N_HDR;
    m_pB[0] = PB_BEGIN;
    m_pB[1] = PC_STREAM;
    
	for (int i = 0; i < nP; i++)
	{
        Eigen::Vector3d vP = pOut->points_[i];
        Eigen::Vector3d vC = pOut->colors_[i];
        Eigen::Vector3d vN = pOut->normals_[i];
        
        pack_int16(&m_pB[iB], (int16_t)(vP.x() * PC_SCALE), false);
        iB += 2;
        pack_int16(&m_pB[iB], (int16_t)(vP.y() * PC_SCALE), false);
        iB += 2;
        pack_int16(&m_pB[iB], (int16_t)(vP.z() * PC_SCALE), false);
        iB += 2;

        pack_int16(&m_pB[iB], (int16_t)(vC.x() * PC_SCALE), false);
        iB += 2;
        pack_int16(&m_pB[iB], (int16_t)(vC.y() * PC_SCALE), false);
        iB += 2;
        pack_int16(&m_pB[iB], (int16_t)(vC.z() * PC_SCALE), false);
        iB += 2;

        if(iB + 12 >= m_nB)
        {
            m_pB[2] = iB - PB_N_HDR;
            m_pIO->write(m_pB, PB_N_HDR + (int)m_pB[2]);

            iB = PB_N_HDR;
        }
    }
    
    if(iB > PB_N_HDR )
    {
        m_pB[2] = iB - PB_N_HDR;
        m_pIO->write(m_pB, PB_N_HDR + (int)m_pB[2]);
    }

    
    //frame sync
    m_pB[0] = PB_BEGIN;
    m_pB[1] = PC_FRAME_END;
    m_pB[2] = 0;
    m_pIO->write(m_pB, PB_N_HDR + (int)m_pB[2]);
    
}

}
#endif
