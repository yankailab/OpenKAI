/*
 * UI.h
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#ifndef SRC_UI_UI_H_
#define SRC_UI_UI_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Stream/_Stream.h"

#define NUM_BUTTON 16

namespace kai
{

struct BUTTON
{
	Rect	m_area;
	string	m_name;
};

class UI
{
public:
	UI();
	virtual ~UI();

	bool init(Config* pConfig, string name);
	bool draw(Frame* pFrame, iVector4* pTextPos);

	void handleMouse(int event, int x, int y, int flags);
	void handleKey(int key);


public:
	int		m_bActiveBtn;
	int		m_eventBtn;

	int 	m_nBtn;
	BUTTON	m_pBtn[NUM_BUTTON];


};

} /* namespace kai */

#endif /* SRC_UI_UI_H_ */
