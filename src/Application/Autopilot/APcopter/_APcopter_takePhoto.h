
#ifndef OpenKAI_src_Autopilot_APcopter__APcopter_takePhoto_H_
#define OpenKAI_src_Autopilot_APcopter__APcopter_takePhoto_H_

#include "../../../Base/common.h"
#include "../../../Vision/_RealSense.h"
#include "../../../IO/_IOBase.h"
#include "../../../Navigation/Coordinate.h"
#include "_APcopter_base.h"

#ifdef USE_REALSENSE

namespace kai
{

class _APcopter_takePhoto: public _AutopilotBase
{
public:
	_APcopter_takePhoto();
	~_APcopter_takePhoto();

	bool init(void* pKiss);
	void update(void);
	int check(void);
	void draw(void);

private:
	void take(void);
	void cmd(void);

	_APcopter_base* m_pAP;
	_VisionBase* m_pV;
	_RealSense* m_pDV;
	_IOBase* m_pIO;

	string m_dir;
	string m_subDir;

	bool m_bAuto;
	bool m_bFlipRGB;
	bool m_bFlipD;
	bool m_bCont;

	vDouble3 m_vGPSoffset;

	uint64_t m_tInterval;
	uint64_t m_tLastTake;
	int m_iTake;

	vector<int> m_compress;
	int m_quality;

};

}

#endif
#endif
