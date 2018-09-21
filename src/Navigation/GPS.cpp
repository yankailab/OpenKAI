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
}

GPS::~GPS()
{
}

UTM_POS GPS::getAbsPos(UTM_POS& pOrigin, vDouble3 dNED)
{
	double hdgRad = pOrigin.m_hdg * DEG_RAD;
	double sinH = sin(hdgRad);
	double cosH = cos(hdgRad);

	UTM_POS pUTM = pOrigin;
	pUTM.m_northing += dNED.x * cosH - dNED.y * sinH;
	pUTM.m_easting += dNED.y * cosH + dNED.x * sinH;
	pUTM.m_alt -= dNED.z;

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
