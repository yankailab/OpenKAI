/*
 * BASE.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_Base_BASE_H_
#define OpenKAI_src_Base_BASE_H_

#include "../Vision/Frame.h"
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
	virtual bool draw(void);
	virtual void reset(void);

	string* getName(void);
	string* getClass(void);

public:
	void*	m_pKiss;
	void*	m_pWindow;
	bool	m_bLog;
	bool	m_bDraw;
};

}

#endif
