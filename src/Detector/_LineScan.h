/*
 * _LineScan.h
 *
 *  Created on: June 2, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__LineScan_H_
#define OpenKAI_src_Detector__LineScan_H_

#include "../Base/common.h"
#include "_DNNclassifier.h"
#include "../Vision/_DepthVisionBase.h"

namespace kai
{

class _LineScan : public _DetectorBase
{
public:
	_LineScan();
	virtual ~_LineScan();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	bool draw(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_LineScan*) This)->update();
		return NULL;
	}

public:
	_DNNclassifier* m_pC;
	_DepthVisionBase* m_pD;

	Mat		m_mR;
	float	m_dThr;
	float	m_w;
	float	m_dW;
	int		m_nW;
	float	m_minArea;
};
}

#endif
