/*
 * _GPS.cpp
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#include "_GPS.h"

namespace kai
{

_GPS::_GPS()
{
	m_pMavlink = NULL;
	m_pSF40 = NULL;
	m_LL.init();
	m_UTM.init();
	m_mavDSfreq = 30;
	m_tLastMavGPS = 0;
}

_GPS::~_GPS()
{
}

bool _GPS::init(void* pKiss)
{
	CHECK_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("mavDSfreq", &m_mavDSfreq));

	return true;
}

bool _GPS::link(void)
{
	CHECK_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_INFO(pK->v("_Lightware_SF40", &iName));
	m_pSF40 = (_Lightware_SF40*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_INFO(pK->v("_Mavlink", &iName));
	m_pMavlink= (_Mavlink*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _GPS::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _GPS::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _GPS::detect(void)
{
	NULL_(m_pSF40);
	NULL_(m_pMavlink);


}

void _GPS::bMavOutput(bool bOutput)
{
	m_bMavOutput = bOutput;
}

void _GPS::mavOutput(void)
{
	//tell apm on/off using HDOP


}

void _GPS::updateMavGPS(void)
{
	//hdg included?
	m_pMavlink->requestDataStream(MAV_DATA_STREAM_POSITION, m_mavDSfreq);

	double base = 1.0/1e7;
	LL_POS llPos;
	llPos.m_lat = ((double)m_pMavlink->m_msg.global_position_int.lat) * base;
	llPos.m_lng = ((double)m_pMavlink->m_msg.global_position_int.lon) * base;
	llPos.m_hdg = ((double)m_pMavlink->m_msg.global_position_int.hdg) * 0.01;

	setLL(&llPos);
}

void _GPS::setLL(LL_POS* pLL)
{
	NULL_(pLL);
	m_LL = *pLL;

	char pUTMzone[8];
	UTM::LLtoUTM(m_LL.m_lat, m_LL.m_lng, m_UTM.m_northing, m_UTM.m_easting, pUTMzone);
	m_UTM.m_zone = pUTMzone;
}

void _GPS::setUTM(UTM_POS* pUTM)
{
	NULL_(pUTM);
	m_UTM = *pUTM;

	UTM::UTMtoLL(m_UTM.m_northing, m_UTM.m_easting, m_UTM.m_zone.c_str(), m_LL.m_lat, m_LL.m_lng);
}

LL_POS* _GPS::getLL(void)
{
	return &m_LL;
}

UTM_POS* _GPS::getUTM(void)
{
	return &m_UTM;
}

bool _GPS::draw(void)
{
	CHECK_F(!this->_ThreadBase::draw());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->getCMat();
	CHECK_F(pMat->empty());

	return true;
}

}
