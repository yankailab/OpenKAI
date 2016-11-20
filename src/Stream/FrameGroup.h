/*
 * FrameGroup.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_STREAM_FRAMEGROUP_H_
#define SRC_STREAM_FRAMEGROUP_H_

#include "../Base/common.h"
#include "../Stream/Frame.h"

#define NUM_FRAME 10

namespace kai
{

class FrameGroup
{
public:
	FrameGroup();
	virtual ~FrameGroup();

	bool init(int numFrame);

	void updateFrameIndex(void);
	Frame* getPrevFrame(void);
	Frame* getLastFrame(void);

private:
	int		m_numFrame;
	int 	m_iFrame;
	Frame* m_pFrame[NUM_FRAME];
};

}/*namespace kai*/

#endif
