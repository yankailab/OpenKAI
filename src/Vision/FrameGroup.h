/*
 * FrameGroup.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_FrameGroup_H_
#define OpenKAI_src_Vision_FrameGroup_H_

#include "../Base/common.h"
#include "Frame.h"

#define FRAMGROUP_N_FRAME 2

namespace kai
{

class FrameGroup
{
public:
	FrameGroup();
	virtual ~FrameGroup();

	bool init(int nFrame);

	void updateFrameIndex(void);
	Frame* getPrevFrame(void);
	Frame* getLastFrame(void);

private:
	int		m_nFrame;
	int 	m_iFrame;
	Frame   m_pFrame[FRAMGROUP_N_FRAME];
};

}

#endif
