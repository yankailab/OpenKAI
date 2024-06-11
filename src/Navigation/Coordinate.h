/*
 * Coordinate.h
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation_GPS_H_
#define OpenKAI_src_Navigation_GPS_H_

#include "../Dependencies/UTM.h"
#include "../Protocol/_Mavlink.h"

/*
APM setting to use MAV_GPS
GPS_TYPE = MAV (not GPS2_TYPE)
EK2_GPS_TYPE = 2
BRD_SER2_RTSCTS = 0
SERIAL2_BAUD = 115
SERIAL2_PROTOCOL = 1
*/

namespace kai
{

#define R_EARTH 6371230
#define OV_R_EARTH 0.000000157

	struct LL_POS
	{
		double m_lat;
		double m_lng;
		double m_altRel;
		double m_altAbs;
		double m_hdg;

		void init(void)
		{
			m_lat = 0.0;
			m_lng = 0.0;
			m_altRel = 0.0;
			m_altAbs = 0.0;
			m_hdg = 0.0;
		}
	};

	struct UTM_POS
	{
		double m_easting;
		double m_northing;
		double m_altRel;
		double m_altAbs;
		double m_hdg;
		string m_zone;

		void init(void)
		{
			m_easting = 0.0;
			m_northing = 0.0;
			m_altRel = 0.0;
			m_altAbs = 0.0;
			m_hdg = 0.0;
			m_zone = "";
		}

		double dist(UTM_POS *pPos)
		{
			if (pPos == NULL)
				return -1.0;

			double e = pPos->m_easting - this->m_easting;
			double n = pPos->m_northing - this->m_northing;
			double a = pPos->m_altRel - this->m_altRel;

			return sqrt(e * e + n * n + a * a);
		}

		UTM_POS operator+(UTM_POS dPos)
		{
			UTM_POS pos;
			pos.init();

			pos.m_easting = this->m_easting + dPos.m_easting;
			pos.m_northing = this->m_northing + dPos.m_northing;
			pos.m_altRel = this->m_altRel + dPos.m_altRel;
			pos.m_altAbs = this->m_altAbs + dPos.m_altAbs;
			pos.m_zone = this->m_zone;
			pos.m_hdg = Hdg(this->m_hdg + dPos.m_hdg);

			return pos;
		}

		UTM_POS operator-(UTM_POS rPos)
		{
			UTM_POS dPos;
			dPos.init();

			dPos.m_easting = this->m_easting - rPos.m_easting;
			dPos.m_northing = this->m_northing - rPos.m_northing;
			dPos.m_altRel = this->m_altRel - rPos.m_altRel;
			dPos.m_altAbs = this->m_altAbs - rPos.m_altAbs;
			dPos.m_zone = this->m_zone;
			dPos.m_hdg = dHdg(rPos.m_hdg, this->m_hdg);

			return dPos;
		}
	};

	class Coordinate
	{
	public:
		Coordinate(void);
		virtual ~Coordinate();

		UTM_POS LL2UTM(LL_POS &pLL);
		LL_POS UTM2LL(UTM_POS &pUTM);

		UTM_POS offset(UTM_POS &UTM, vFloat3 &dNEA);
		LL_POS offset(LL_POS &LL, vFloat3 &dNEA);
	};

}
#endif
