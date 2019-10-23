/*
 * Console.h
 *
 *  Created on: Oct 13, 2019
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_Console_H_
#define OpenKAI_src_UI_Console_H_

#include "../Base/common.h"
#include "../Base/BASE.h"

namespace kai
{

class Console: public BASE
{
public:
	Console();
	virtual ~Console();

	bool init(void *pKiss);
	void draw(void);
	void addMsg(const string &msg, int iCol, int iX, int iLine = 0);

public:
	int m_iY;

};

}
#endif
