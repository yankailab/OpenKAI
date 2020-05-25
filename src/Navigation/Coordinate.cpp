/*
 * Coordinate.cpp
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#include "Coordinate.h"

namespace kai
{

Coordinate::Coordinate()
{
}

Coordinate::~Coordinate()
{
}

UTM_POS Coordinate::offset(UTM_POS& UTM, vFloat3& dNEA)
{
	double hdgRad = UTM.m_hdg * DEG_2_RAD;
	double sinH = sin(hdgRad);
	double cosH = cos(hdgRad);

	UTM_POS pUTM = UTM;
	pUTM.m_northing += dNEA.x * cosH - dNEA.y * sinH;
	pUTM.m_easting += dNEA.y * cosH + dNEA.x * sinH;
	pUTM.m_altRel += dNEA.z;
	pUTM.m_altAbs += dNEA.z;

	return pUTM;
}

LL_POS Coordinate::offset(LL_POS& LL, vFloat3& dNEA)
{
	//http://www.edwilliams.org/avform.htm#LL

	LL_POS oLL = LL;
	double d = sqrt(dNEA.x*dNEA.x + dNEA.y*dNEA.y) * OV_R_EARTH;
	double tc = oLL.m_hdg*DEG_2_RAD + atan2(dNEA.y, dNEA.x);	//true course

	double sinLat = sin(LL.m_lat);
	double cosLat = cos(LL.m_lat);

	oLL.m_lat = asin(sinLat * cos(d) + cosLat * sin(d) * cos(tc));
	double dLon = atan2(sin(tc) * sin(d) * cosLat, cos(d) - sinLat * sin(oLL.m_lat));
	oLL.m_lng = fmod(LL.m_lng + dLon + CV_PI, 2*CV_PI) - CV_PI;

	return oLL;
}

UTM_POS Coordinate::LL2UTM(LL_POS& pLL)
{
	UTM_POS pUTM;
	char pUTMzone[UTM_BUF];
	UTM::LLtoUTM(pLL.m_lat, pLL.m_lng, pUTM.m_northing, pUTM.m_easting,
			pUTMzone);
	pUTM.m_zone = pUTMzone;
	pUTM.m_altRel = pLL.m_altRel;
	pUTM.m_altAbs = pLL.m_altAbs;
	pUTM.m_hdg = pLL.m_hdg;

	return pUTM;
}

LL_POS Coordinate::UTM2LL(UTM_POS& pUTM)
{
	LL_POS pLL;
	UTM::UTMtoLL(pUTM.m_northing, pUTM.m_easting, pUTM.m_zone.c_str(),
			pLL.m_lat, pLL.m_lng);
	pLL.m_altRel = pUTM.m_altRel;
	pLL.m_altAbs = pUTM.m_altAbs;
	pLL.m_hdg = pUTM.m_hdg;

	return pLL;
}

}
