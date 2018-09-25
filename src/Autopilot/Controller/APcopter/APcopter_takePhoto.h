
#ifndef OpenKAI_src_Autopilot_Controller_APcopter_takePhoto_H_
#define OpenKAI_src_Autopilot_Controller_APcopter_takePhoto_H_

#include "../../../Base/common.h"
#include "../../../Vision/_DepthVisionBase.h"
#include "APcopter_base.h"

namespace kai
{

class APcopter_takePhoto: public ActionBase
{
public:
	APcopter_takePhoto();
	~APcopter_takePhoto();

	bool init(void* pKiss);
	void update(void);

private:
	APcopter_base* m_pAP;
	_DepthVisionBase* m_pDV;

	string m_dir;
	string m_subDir;
	uint64_t m_tInterval;
	uint64_t m_tLastTake;

	vector<int> m_PNGcompress;
	int m_PNGcompression;

};

}

#endif
