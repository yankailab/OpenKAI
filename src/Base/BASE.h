/*
 * BASE.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_BASE_BASE_H_
#define OPENKAI_SRC_BASE_BASE_H_

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

	string* getName(void);
	string* getClass(void);

	virtual bool start(void);
	virtual bool draw(void);
	virtual void complete(void);

public:
	void*	m_pKiss;
	void*	m_pWindow;
	bool	m_bLog;
	bool	m_bDraw;
};

}

#endif
