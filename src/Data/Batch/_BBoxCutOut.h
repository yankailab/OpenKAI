/*
 * _BBoxCutOut.h
 *
 *  Created on: Sept 16, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Batch__BBoxCutOut_H_
#define OpenKAI_src_Data_Batch__BBoxCutOut_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"

namespace kai
{

class _BBoxCutOut: public _ThreadBase
{
public:
	_BBoxCutOut();
	~_BBoxCutOut();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void process();
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_BBoxCutOut*) This)->update();
		return NULL;
	}

public:
	string m_dirIn;
	string m_dirOut;
	string m_extTxt;
	string m_extImgIn;
	string m_extImgOut;


};
}

#endif
