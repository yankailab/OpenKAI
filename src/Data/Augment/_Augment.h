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
#include "../_DataBase.h"

namespace kai
{

class _Augment: public _DataBase
{
public:
	_Augment();
	~_Augment();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void rotate(void);
	void move(void);
	void scaling(void);
	void crop(void);
	void flip(void);
	void tone(void);
	void noise(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Augment*) This)->update();
		return NULL;
	}

public:
	bool m_bRot;
	int m_nRot;
	int m_rotNoiseMean;
	int m_rotNoiseDev;

	bool m_bFlip;
	bool m_bScaling;
	double m_dScaling;
	int m_nScaling;

	int m_noiseMean;
	int m_noiseDev;
	int m_nNoise;

	bool m_bDeleteOriginal;
	double m_progress;

};
}

#endif
