/*
 * _BBoxCutOut.h
 *
 *  Created on: Sept 16, 2017
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_DNN__BBoxCutOut_H_
#define OPENKAI_SRC_DNN__BBoxCutOut_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Utility/util.h"

namespace kai
{

class _BBoxCutOut: public _ThreadBase
{
public:
	_BBoxCutOut();
	~_BBoxCutOut();

	bool init(void* pKiss);
	bool link(void);

private:
	void process();
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_BBoxCutOut*) This)->update();
		return NULL;
	}

};
}

#endif
