/*
 * CameraInput.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_FRAMEGROUP_H_
#define SRC_FRAMEGROUP_H_

#include <pthread.h>
#include "stdio.h"
#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Utility/util.h"
#include "Frame.h"

#define NUM_FRAME 10

namespace kai
{

class FrameGroup
{
public:
	FrameGroup();
	virtual ~FrameGroup();

	bool init(int numFrame);
//	bool addFrame(CamFrame* pFrame, int iFrame);

	void updateFrameIndex(void);
	Frame* getPrevFrame(void);
	Frame* getLastFrame(void);

//	void switchFrame(void);
//	void updateSwitch(Mat* pFrame);
//	void updateSwitch(CamFrame* pFrame);

private:
	int		m_numFrame;
	int 	m_iFrame;
	Frame* m_pFrame[NUM_FRAME];
};

}/*namespace kai*/

#endif /* SRC_FRAMEGROUP_H_ */
