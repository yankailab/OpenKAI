/*
 * _FilterBase.h
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__FilterBase_H_
#define OpenKAI_src_Data_Augment__FilterBase_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "../_DataBase.h"

namespace kai
{

class _FilterBase: public _DataBase
{
public:
	_FilterBase();
	~_FilterBase();

	bool init(void* pKiss);
	bool start(void);

	bool bComplete(void);

	virtual void update(void);
	static void* getUpdateThread(void* This)
	{
		((_FilterBase*) This)->update();
		return NULL;
	}

public:
	bool m_bComplete;
	int m_nProduce;
	double m_progress;
	double m_dRand;

	int m_noiseMean;
	int m_noiseDev;
	int m_noiseType;
};
}
#endif
