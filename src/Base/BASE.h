/*
 * BASE.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_Base_BASE_H_
#define OpenKAI_src_Base_BASE_H_

#include "common.h"

using namespace std;

namespace kai
{

class BASE
{
public:
	BASE();
	virtual ~BASE();

	virtual bool init(void* pKiss);
	virtual bool link(void);
	virtual bool start(void);
	virtual int check(void);
	virtual void cvDraw(void* pWindow);
	virtual void console(void* pConsole);
	virtual int serialize(uint8_t* pB, int nB);
	virtual int deSerialize(uint8_t* pB, int nB);

	string* getName(void);
	string* getClass(void);

public:
	void*	m_pKiss;
	bool	m_bLog;
	bool	m_bDebug;
};

}

#endif
