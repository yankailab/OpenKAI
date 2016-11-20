/*
 * CameraOpticalFlow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef SRC_3DFlow_H_
#define SRC_3DFlow_H_

#include "../Base/common.h"
#include "../Vision/_Flow.h"
#include "../Stream/Frame.h"
#include "../Detector/DetectorBase.h"

using namespace cv;
using namespace cv::cuda;
using namespace std;

namespace kai
{

class _3DFlow:  public DetectorBase, public _ThreadBase
{
public:
	_3DFlow();
	virtual ~_3DFlow();

	bool init(JSON* pJson, string name);
	bool start(void);

private:
	void findDepth(void);
	void findDepthGPU(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_3DFlow*) This)->update();
		return NULL;
	}

public:
	_Flow*	m_pDF;
	Rect	m_ROI;
	double	m_flowMax;
	double	m_flowAvr; //average flow distance in previous frame
	int		m_targetArea;

	Frame* m_pDepth;
	Frame* m_pSeg;

	Mat m_labelColor;
	Ptr<LookUpTable>	m_pGpuLUT;

};

} /* namespace kai */

#endif /* SRC_3DFlow_H_ */
