
#ifndef OPENKAI_SRC_AUTOPILOT_ACTION_APCOPTER_VISUALAVOID_H_
#define OPENKAI_SRC_AUTOPILOT_ACTION_APCOPTER_VISUALAVOID_H_

#include "../../../Base/common.h"
#include "../../../AI/_MatrixNet.h"
#include "../ActionBase.h"
#include "../APcopter/APcopter_base.h"

#define N_TERRAIN 16

namespace kai
{

struct TERRAIN
{
	uint16_t m_iClass;
	uint8_t	m_action;
	uint8_t m_orientation;
	vDouble4 m_roi;

	void init(void)
	{
		m_iClass = 0;
		m_action = 0;
		m_orientation = 0;
		m_roi.x = 0.0;
		m_roi.y = 0.0;
		m_roi.z = 1.0;
		m_roi.w = 1.0;
	}
};

class APcopter_visualAvoid: public ActionBase
{
public:
	APcopter_visualAvoid();
	~APcopter_visualAvoid();

	bool init(void* pKiss);
	bool link(void);
	void update(void);
	bool draw(void);

private:
	APcopter_base* m_pAP;
	_MatrixNet*	m_pMN;

	TERRAIN m_pTerrain[N_TERRAIN];
	int m_nTerrain;

};

}

#endif

