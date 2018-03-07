/*
 * _MultiImageNet.h
 *
 *  Created on: Oct 26, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Inference__MultiImageNet_H_
#define OpenKAI_src_Data_Inference__MultiImageNet_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Classifier/_ImageNet.h"
#include "../_DataBase.h"

namespace kai
{

class _MultiImageNet: public _DataBase
{
public:
	_MultiImageNet();
	~_MultiImageNet();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void inference(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_MultiImageNet*) This)->update();
		return NULL;
	}

public:
#ifdef USE_TENSORRT
	_ImageNet* m_pIN1;
	_ImageNet* m_pIN2;
	_ImageNet* m_pIN3;
#endif

	Frame* m_pFrame;

	double m_progress;

};
}

#endif
