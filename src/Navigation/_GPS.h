/*
 * _GPS.h
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#ifndef SRC_NAVIGATION_GPS_H_
#define SRC_NAVIGATION_GPS_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Filter/FilterBase.h"
#include "../Sensor/_Lightware_SF40.h"
#include "../Protocol/_Mavlink.h"
#include "../include/UTM.h"

namespace kai
{

struct LL_POS
{
	double	m_lat;
	double	m_lng;
	double	m_hdg;

	void init(void)
	{
		m_lat = 0.0;
		m_lng = 0.0;
		m_hdg = 0.0;
	}
};

struct UTM_POS
{
	double m_easting;
	double m_northing;
	string m_zone;

	void init(void)
	{
		m_easting = 0.0;
		m_northing = 0.0;
		m_zone = "";
	}
};

class _GPS: public _ThreadBase
{
public:
	_GPS(void);
	virtual ~_GPS();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	void setLL(LL_POS* pLL);
	void setUTM(UTM_POS* pUTM);
	LL_POS* getLL(void);
	UTM_POS* getUTM(void);

private:
	void setMavGPS(void);
	void getMavGPS(void);
	bool updateLidar(void);
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_GPS *) This)->update();
		return NULL;
	}

public:
	_Lightware_SF40* m_pSF40;
	_Mavlink* m_pMavlink;
	int	m_mavDSfreq;
	uint32_t m_apmMode;

	uint64_t m_tStarted;
	uint64_t m_time;

	LL_POS	m_initLL;
	LL_POS	m_LL;
	UTM_POS m_UTM;

};

}

#endif
