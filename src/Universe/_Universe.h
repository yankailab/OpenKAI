/*
 * _Universe.h
 *
 *  Created on: June 21, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe__Universe_H_
#define OpenKAI_src_Universe__Universe_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "_Object.h"

#define UNIVERSE_N_O 256

namespace kai
{

struct _OBJECT_ARRAY
{
	_Object* m_ppO[UNIVERSE_N_O];
	int m_nO;

	void reset(void)
	{
		m_nO = 0;
	}

	int add(_Object* pO)
	{
		NULL__(pO, -1);
		IF__(m_nO >= UNIVERSE_N_O, -1);

		m_ppO[m_nO++] = pO;

		return m_nO-1;
	}

	_Object* at(int i)
	{
		IF_N(i >= m_nO);
		IF_N(i < 0);
		return m_ppO[i];
	}

	int size(void)
	{
		return m_nO;
	}
};

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
	virtual int add(_Object* pO);
	virtual _Object* get(int i);

	//pipeline
	virtual void pipeIn(void);

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
	_OBJECT_ARRAY	m_pO[2];
	_OBJECT_ARRAY*	m_pPrev;
	_OBJECT_ARRAY*	m_pNext;

	//config
	float m_minConfidence;
	float m_minArea;
	float m_maxArea;
	float m_minW;
	float m_maxW;
	float m_minH;
	float m_maxH;
	float m_bbScale;

	//draw
	bool m_bDrawStatistics;
	bool m_bDrawClass;

};

}
#endif
