
#ifndef OpenKAI_src_Autopilot_Controller_Generic_hiphenRGB_H_
#define OpenKAI_src_Autopilot_Controller_Generic_hiphenRGB_H_

#include "../../../Base/common.h"
#include "../../../IO/_IOBase.h"
#include "../../../Vision/_VisionBase.h"
#include "../../ActionBase.h"

namespace kai
{

class Generic_hiphenRGB: public ActionBase
{
public:
	Generic_hiphenRGB();
	~Generic_hiphenRGB();

	bool init(void* pKiss);
	void update(void);
	int check(void);
	bool draw(void);
	bool console(int& iY);

private:
	void take(void);

	_VisionBase* m_pV;
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
