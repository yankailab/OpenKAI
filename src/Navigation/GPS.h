/*
 * GPS.h
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation_GPS_H_
#define OpenKAI_src_Navigation_GPS_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Dependency/UTM.h"
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

struct LL_POS
{
	double	m_lat;
	double	m_lng;
	double	m_alt;
	double	m_hdg;

	void init(void)
	{
		m_lat = 0.0;
		m_lng = 0.0;
		m_alt = 0.0;
		m_hdg = 0.0;
	}
};

struct UTM_POS
{
	double m_easting;
	double m_northing;
	double m_alt;
	double m_hdg;
	string m_zone;

	void init(void)
	{
		m_easting = 0.0;
		m_northing = 0.0;
		m_alt = 0.0;
		m_hdg = 0.0;
		m_zone = "";
	}

	inline double dist(UTM_POS* pPos)
	{
		if(pPos==NULL)return -1.0;

		double e = pPos->m_easting - this->m_easting;
		double n = pPos->m_northing - this->m_northing;
		double a = pPos->m_alt - this->m_alt;

		return sqrt(e*e + n*n + a*a);
	}

	inline UTM_POS operator-(UTM_POS rPos)
	{
		UTM_POS dPos;
		dPos.init();

		dPos.m_easting = this->m_easting - rPos.m_easting;
		dPos.m_northing = this->m_northing - rPos.m_northing;
		dPos.m_alt = this->m_alt - rPos.m_alt;
		dPos.m_zone = this->m_zone;
		dPos.m_hdg = dHdg(rPos.m_hdg, this->m_hdg);

		return dPos;
	}

};

class GPS: public BASE
{
public:
	GPS(void);
	virtual ~GPS();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool cli(int& iY);

	bool getMavHdg(void);
	void setMavGPS(void);

	void setRelPos(vDouble3& dPos);
	void setLL(LL_POS* pLL);
	void setUTM(UTM_POS* pUTM);
	LL_POS* getLL(void);
	UTM_POS* getUTM(void);
	LL_POS* getInitLL(void);
	UTM_POS* getInitUTM(void);

public:
	_Mavlink* m_pMavlink;
	int	m_mavDSfreq;

	LL_POS	m_originLL;
	UTM_POS m_originUTM;

	LL_POS	m_LL;
	UTM_POS m_UTM;

	vDouble3 m_vDpos;	//relative pos to the initial LL
	uint8_t m_nSat;
};

}
#endif
