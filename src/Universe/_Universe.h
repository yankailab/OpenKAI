/*
 * _Universe.h
 *
 *  Created on: June 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe__Universe_H_
#define OpenKAI_src_Universe__Universe_H_

#include "_ObjectArray.h"

namespace kai
{

class _Universe: public _ThreadBase
{
public:
	_Universe();
	virtual ~_Universe();

	virtual bool init(void* pKiss);
	virtual bool start(void);
	virtual void update(void);
	virtual void draw(void);

	//io
	virtual _Object* add(_Object& pO);
	virtual _Object* get(int i);

protected:
	virtual void clearObj(void);
	virtual void updateObj(void);

private:
	static void* getUpdateThread(void* This)
	{
		((_Universe *) This)->update();
		return NULL;
	}

public:
	//general
	uint8_t	m_nDim;

	//data
	int m_iSwitch;
	_ObjectArray	m_pO[2];
	_ObjectArray*	m_pPrev;
	_ObjectArray*	m_pNext;

	//config
	float m_minConfidence;
	float m_minArea;
	float m_maxArea;
	float m_minW;
	float m_maxW;
	float m_minH;
	float m_maxH;
	float m_bbScale;
	bool m_bMerge;
	float m_mergeOverlap;
	vFloat4 m_vRoi;

	//draw
	bool m_bDrawStatistics;
	bool m_bDrawClass;

};

}
#endif
