/*
 * _ClusterNet.h
 *
 *  Created on: Aug 17, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__ClusterNet_H_
#define OpenKAI_src_Detector__ClusterNet_H_

#include "../Base/common.h"
#include "../DNN/TensorRT/_ImageNet.h"
#include "_DetectorBase.h"

namespace kai
{

class _ClusterNet: public _DetectorBase
{
public:
	_ClusterNet();
	~_ClusterNet();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	void bSetActive(bool bActive);
	OBJECT* get(int i);
	bool bFound(int iClass);

private:
	vInt4 explore(int x, int y, int iClass);
	void cluster(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_ClusterNet*) This)->update();
		return NULL;
	}

public:
	_DetectorBase* m_pDet;

	int m_nObj;
	double m_w;
	double m_h;
	double m_dW;
	double m_dH;
	double m_aW;
	double m_aH;
	vInt2 m_size;
	vDouble4 m_area;
	OBJECT** m_ppObj;

};

}

#endif
