/*
 * _DepthEdge.h
 *
 *  Created on: Sept 14, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__DepthEdge_H_
#define OpenKAI_src_Detector__DepthEdge_H_

#include "../Base/common.h"
#include "../Base/_ObjectBase.h"
#include "../Arithmetic/PolyFit.h"
#include "../Vision/_DepthVisionBase.h"

namespace kai
{

class _DepthEdge : public _ThreadBase
{
public:
	_DepthEdge();
	virtual ~_DepthEdge();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool cli(int& iY);

	vDouble4* pos(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DepthEdge*) This)->update();
		return NULL;
	}


public:
	_DepthVisionBase* m_pDV;

	int m_wD;
	int m_hD;
	vDouble4 m_vPos;

	vDouble4 m_vBBedge;
	vDouble4 m_vBBL;
	vDouble4 m_vBBR;
	double m_dL;
	double m_dR;

	int m_nMorphOpen;
	int m_nMorphClose;
	int m_sobelK;

	PolyFit m_pf;
	Mat m_mF;

};

}
#endif
