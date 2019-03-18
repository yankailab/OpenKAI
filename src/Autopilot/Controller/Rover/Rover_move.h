#ifndef OpenKAI_src_Autopilot_Controller_Rover_Rover_move_H_
#define OpenKAI_src_Autopilot_Controller_Rover_Rover_move_H_

#include "../../../Detector/_DetectorBase.h"
#include "../../../Protocol/_Mavlink.h"
#include "Rover_base.h"

namespace kai
{

class Rover_move: public ActionBase
{
public:
	Rover_move();
	~Rover_move();

	bool init(void* pKiss);
	int	 check(void);
	void update(void);
	bool draw(void);
	bool console(int& iY);

	bool findTag(void);

public:
	Rover_base* m_pR;
	_Mavlink* m_pMavlink;
	_DetectorBase*	m_pDet;

	float	m_hdg;
	float	m_nSpeed;

	vFloat4	m_tagRoi;
	int		m_iTag;
	float	m_tagAngle;
	uint8_t	m_iPinLEDtag;
};

}
#endif
