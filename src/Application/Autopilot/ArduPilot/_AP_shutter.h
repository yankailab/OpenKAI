
#ifndef OpenKAI_src_Autopilot_AP__AP_shutter_H_
#define OpenKAI_src_Autopilot_AP__AP_shutter_H_

#include "../../../Base/common.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../../Vision/_GPhoto.h"
#include "../ArduPilot/_AP_base.h"
#include "../../../Detector/_DetectorBase.h"

namespace kai
{

class _AP_shutter: public _AutopilotBase
{
public:
	_AP_shutter();
	~_AP_shutter();

	bool init(void* pKiss);
	bool start(void);
	void update(void);
	int check(void);
	void draw(void);

private:
	void shutter(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_shutter *) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	_VisionBase* m_pV;
	_DepthVisionBase* m_pDV;
	_GPhoto* m_pG;

	string m_dir;
	string m_subDir;
	int m_iTake;
	INTERVAL_EVENT m_ieShutter;

	bool m_bFlipRGB;
	bool m_bFlipD;
	vector<int> m_compress;
	int m_quality;

	_DetectorBase* m_pDet;
	int m_iTag;

};

}

#endif
