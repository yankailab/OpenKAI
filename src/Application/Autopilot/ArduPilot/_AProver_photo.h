
#ifndef OpenKAI_src_Autopilot_AP__AProver_photo_H_
#define OpenKAI_src_Autopilot_AP__AProver_photo_H_

#include "../../../Vision/_DepthVisionBase.h"

#ifdef USE_OPENCV
#include "../../../Vision/_GPhoto.h"
#include "_AP_base.h"
#include "_AProver_drive.h"

namespace kai
{

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
	_AProver_drive* m_pDrive;

	_VisionBase* m_pV;
	_DepthVisionBase* m_pDV;
	_GPhoto* m_pG;

	float	m_nSpeed;
	float	m_yaw;
	int		m_lastAPmode;

	int m_iTake;
	uint64_t m_tDelay;
	uint64_t m_tInterval;

	string m_dir;
	string m_subDir;
	bool m_bFlipRGB;
	bool m_bFlipD;
	vector<int> m_compress;
	int m_quality;

};

}
#endif
#endif
