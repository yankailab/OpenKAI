
#ifndef OpenKAI_src_Autopilot_AP__AProver_photo_H_
#define OpenKAI_src_Autopilot_AP__AProver_photo_H_

#include "../../../Base/common.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../../Vision/_GPhoto.h"
#include "../../../Detector/_DetectorBase.h"
#include "../../../Control/PIDctrl.h"
#include "../ArduPilot/_AP_base.h"
#include "_AProver_drive.h"

namespace kai
{

enum AP_PHOTO_MODE
{
	apShutter_det = 0,
	apShutter_cont = 1,
	apShutter_manual = 2,
};

class _AProver_photo: public _AutopilotBase
{
public:
	_AProver_photo();
	~_AProver_photo();

	bool init(void* pKiss);
	bool start(void);
	void update(void);
	int check(void);
	void draw(void);

private:
	void shutter(void);
	static void* getUpdateThread(void* This)
	{
		((_AProver_photo *) This)->update();
		return NULL;
	}

private:
	_AP_base* m_pAP;
	PIDctrl* m_pPIDhdg;
	_AProver_drive* m_pDrive;
	_DetectorBase* m_pDet;
	vFloat4	m_vTargetBB;

	_VisionBase* m_pV;
	_DepthVisionBase* m_pDV;
	_GPhoto* m_pG;

	float	m_nSpeed;
	float	m_dHdg;
	float	m_xTarget;

	string m_dir;
	string m_subDir;
	bool m_bFlipRGB;
	bool m_bFlipD;
	vector<int> m_compress;
	int m_quality;

	int m_iTake;
	AP_PHOTO_MODE m_shutterMode;
	INTERVAL_EVENT m_ieShutter;
	int m_iTag;
	uint64_t m_tDelay;

//	bool	 m_bModeChange;
//	uint32_t m_apModeShutter;

};

}

#endif
