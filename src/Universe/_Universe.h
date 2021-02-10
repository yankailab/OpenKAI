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

struct OBJ_CLASS
{
	string m_name;
	int m_n;

	void init(void)
	{
		m_n = 0;
		m_name = "";
	}
};

class _Universe: public _ModuleBase
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
	virtual int size(void);

	virtual void clearObj(void);
	virtual void updateObj(void);

	virtual void updateStatistics(void);

private:
	static void* getUpdate(void* This)
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
	vFloat2 m_rArea;
	vFloat2 m_rW;
	vFloat2 m_rH;
	float m_bbExpand;
	bool m_bMerge;
	float m_mergeOverlap;
	vFloat4 m_vRoi;
    vInt2 m_vClassRange;

	//show
	bool m_bDrawStatistics;
	vInt3 m_classLegendPos;
	bool m_bDrawClass;
	bool m_bDrawText;
	bool m_bDrawPos;
};

}
#endif
