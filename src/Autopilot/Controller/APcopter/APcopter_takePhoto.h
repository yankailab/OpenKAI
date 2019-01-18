
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_takePhoto_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_takePhoto_H_

#include "../../../Base/common.h"
#include "../../../Vision/_RealSense.h"
#include "../../../IO/_IOBase.h"
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
	bool draw(void);
	bool console(int& iY);

private:
	void take(void);
	void cmd(void);

	APcopter_base* m_pAP;
	_VisionBase* m_pV;
	_RealSense* m_pDV;
	_IOBase* m_pIO;

	string m_dir;
	string m_subDir;

	bool m_bAuto;
	bool m_bFlipRGB;
	bool m_bFlipD;

	uint64_t m_tInterval;
	uint64_t m_tLastTake;
	int m_iTake;

	vector<int> m_compress;
	int m_quality;

};

}

#endif
#endif
