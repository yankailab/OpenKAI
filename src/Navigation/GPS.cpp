/*
 * GPS.cpp
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#include "GPS.h"

namespace kai
{

GPS::GPS()
{
	m_LL.init();
	m_UTM.init();
}

GPS::~GPS()
{
}

void GPS::update(_Mavlink* pMav)
{
	NULL_(pMav);

	m_LL.m_lat = pMav->m_msg.global_position_int.lat * 0.0000001;
	m_LL.m_lng = pMav->m_msg.global_position_int.lon * 0.0000001;
	m_LL.m_alt = (double)pMav->m_msg.global_position_int.relative_alt * 0.001;
	m_LL.m_hdg = pMav->m_msg.global_position_int.hdg * 0.01;

	update(m_LL);
}

void GPS::update(LL_POS& pLL)
{
	m_LL = pLL;
	m_UTM = LL2UTM(pLL);
}

void GPS::update(UTM_POS& pUTM)
{
	m_UTM = pUTM;
	m_LL = UTM2LL(m_UTM);
}

UTM_POS GPS::getUTM(void)
{
	return m_UTM;
}

LL_POS GPS::getLL(void)
{
	return m_LL;
}

UTM_POS GPS::getPos(vDouble3& dNEA)
{
	double hdgRad = m_UTM.m_hdg * DEG_RAD;
	double sinH = sin(hdgRad);
	double cosH = cos(hdgRad);

	UTM_POS pUTM = m_UTM;
	pUTM.m_northing += dNEA.x * cosH - dNEA.y * sinH;
	pUTM.m_easting += dNEA.y * cosH + dNEA.x * sinH;
	pUTM.m_alt += dNEA.z;

	return pUTM;
}

UTM_POS GPS::LL2UTM(LL_POS& pLL)
{
	UTM_POS pUTM;
	char pUTMzone[8];
	UTM::LLtoUTM(pLL.m_lat, pLL.m_lng, pUTM.m_northing, pUTM.m_easting, pUTMzone);
	pUTM.m_zone = pUTMzone;
	pUTM.m_alt = pLL.m_alt;
	pUTM.m_hdg = pLL.m_hdg;

	return pUTM;
}

LL_POS GPS::UTM2LL(UTM_POS& pUTM)
{
	LL_POS pLL;
	UTM::UTMtoLL(pUTM.m_northing, pUTM.m_easting, pUTM.m_zone.c_str(), pLL.m_lat, pLL.m_lng);
	pLL.m_alt = pUTM.m_alt;
	pLL.m_hdg = pUTM.m_hdg;

	return pLL;
}

}
