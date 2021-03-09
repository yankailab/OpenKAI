/*
 * Goto.h
 *
 *  Created on: March 16, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_State_Goto_H_
#define OpenKAI_src_State_Goto_H_

#include "State.h"

namespace kai
{

class Goto: public State
{
public:
	Goto();
	virtual ~Goto();

	bool init(void* pKiss);
	bool update(void);
	void reset(void);
	void draw(void);

	void setPos(vDouble3& p);

private:
	vDouble3 m_vPos;	//lat, lon, alt
	double	 m_hdg;
};

}
#endif
