/*
 * CamMonitor.h
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMMONITOR_H_
#define SRC_CAMMONITOR_H_

#include "common.h"
#include "cvplatform.h"

#include "CamFrame.h"
#include "CamMarkerDetect.h"
#include "CamDenseFlow.h"
#include "CamSparseFlow.h"

namespace kai
{

class CamMonitor
{
public:
	CamMonitor();
	virtual ~CamMonitor();

	void setWindowName(string name);
	bool init(void);
	void addFrame(CamFrame* pFrame, int zIndex, double alpha);
	void addMarkerDetect(CamFrame* pFrame, int zIndex, double alpha);
	void addDenseFlow(CamFrame* pFrame, int zIndex, double alpha);
	void addSparseFlow(CamFrame* pFrame, int zIndex, double alpha);
	void addObjectDetect(CamFrame* pFrame, int zIndex, double alpha);

	void show(void);

public:
	string m_windowName;
	Mat m_mat;

};

} /* namespace kai */

#endif /* SRC_CAMMONITOR_H_ */
