/*
 * _imgRotate.h
 *
 *  Created on: Mar 12, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_imgRotate__imgRotate_H_
#define OpenKAI_src_Data_imgRotate__imgRotate_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"
#include "../_DataBase.h"

namespace kai
{

class _imgRotate: public _DataBase
{
public:
	_imgRotate();
	~_imgRotate();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void process(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_imgRotate*) This)->update();
		return NULL;
	}

public:
	int m_bgNoiseMean;
	int m_bgNoiseDev;
	int m_bgNoiseType;

	int m_nProduce;

	bool m_bDeleteOriginal;
	bool m_bSaveOriginalCopy;
	double m_progress;
};
}

#endif
