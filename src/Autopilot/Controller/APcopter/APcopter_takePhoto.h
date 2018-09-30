
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_takePhoto_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_takePhoto_H_

#include "../../../Base/common.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "../../../Vision/_RealSense.h"
#include "APcopter_base.h"

#ifdef USE_REALSENSE

namespace kai
{

class APcopter_takePhoto: public ActionBase
{
public:
	APcopter_takePhoto();
	~APcopter_takePhoto();

	bool init(void* pKiss);
	void update(void);
	int check(void);

private:
	APcopter_base* m_pAP;
	_VisionBase* m_pV;
	_RealSense* m_pDV;

	string m_dir;
	string m_subDir;
	uint64_t m_tInterval;
	uint64_t m_tLastTake;
	int m_iTake;

	vector<int> m_compress;
	int m_quality;

};

}

#endif
#endif
