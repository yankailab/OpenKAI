#ifndef OpenKAI_src_Autopilot_AP__AP_video_H_
#define OpenKAI_src_Autopilot_AP__AP_video_H_

#include "../../Vision/_DepthVisionBase.h"
#include "../../Vision/_GPhoto.h"
#include "../../Sensor/_DistSensorBase.h"
#include "_AP_base.h"
#include "_AP_posCtrl.h"

namespace kai
{

class _AP_video: public _StateBase
{
public:
	_AP_video();
	~_AP_video();

	bool init(void* pKiss);
	bool start(void);
	void update(void);
	int check(void);
	void console(void* pConsole);

private:
	void autoMode(void);
	void manualMode(void);
	void shutter(void);
	static void* getUpdate(void* This)
	{
		((_AP_video *) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	_AP_posCtrl* m_pPC;
	_DistSensorBase* m_pDS;
	int m_iDiv;
	float m_speed;

	_VisionBase* m_pV;
	_DepthVisionBase* m_pDV;
	_GPhoto* m_pG;

	int		m_iRCshutter;
	STATE_CHANGE m_scShutter;

	float	m_alt;
	float	m_dAlt;
	float	m_lastAlt;
	int		m_iRelayShutter;

	int m_iTake;
	uint64_t m_tDelay;

	string m_dir;
	string m_subDir;
	bool m_bFlipRGB;
	bool m_bFlipD;
	vector<int> m_compress;
	int m_quality;

};

}
#endif
