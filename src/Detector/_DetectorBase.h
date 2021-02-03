/*
 * _DetectorBase.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base__DetectorBase_H_
#define OpenKAI_src_Base__DetectorBase_H_

#include "../Universe/_Universe.h"

#ifdef USE_OPENCV
#include "../Vision/_VisionBase.h"
#include "../Vision/_DepthVisionBase.h"
#include "../Protocol/_Mavlink.h"

namespace kai
{

class _DetectorBase: public _ModuleBase
{
public:
	_DetectorBase();
	virtual ~_DetectorBase();

	virtual bool init(void *pKiss);
	virtual void draw(void);
	virtual int check(void);
	virtual int getClassIdx(string &className);
	virtual string getClassName(int iClass);

public:
	//input
	_VisionBase *m_pV;
	_DepthVisionBase* m_pDV;
	_DetectorBase *m_pDB;

	//data
	Frame m_fBGR;
	_Universe* m_pU;

	//model
	string m_fModel;
	string m_fWeight;
	string m_fMean;
	string m_fClass;
	int m_nClass;
	vector<OBJ_CLASS> m_vClass;

	//show
	bool m_bDrawStatistics;
	vInt3 m_classLegendPos;
	bool m_bDrawClass;
	bool m_bDrawText;
	bool m_bDrawPos;

};

}
#endif
#endif
