/*
 * Land.h
 *
 *  Created on: Dec 18, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Mission_Land_H_
#define OpenKAI_src_Mission_Land_H_

#include "MissionBase.h"

namespace kai
{

class Land: public MissionBase
{
public:
	Land();
	virtual ~Land();

	bool init(void* pKiss);
	bool update(void);
	void draw(void);

	void setLanded(bool bLanded);

public:
	int m_tag;
	double m_hdg;
	double m_speed;

};

}
#endif
