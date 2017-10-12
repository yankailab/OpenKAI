/*
 * _Augment.h
 *
 *  Created on: Oct 12, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data_Augment__Augment_H_
#define OpenKAI_src_Data_Augment__Augment_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Utility/util.h"

namespace kai
{

class _Augment: public _ThreadBase
{
public:
	_Augment();
	~_Augment();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void rorate(void);
	void move(void);
	void scaling(void);
	void crop(void);
	void flip(void);
	void tone(void);

	void process();
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Augment*) This)->update();
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
