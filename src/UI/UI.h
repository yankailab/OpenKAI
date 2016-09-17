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
	string	m_name;
	Rect	m_rect;
	Scalar	m_color;
	double	m_sizeFont;

	int		m_event;

	void handler(int event, int x, int y, int flags)
	{
		m_event = 0;
		if(x<m_rect.x || x>m_rect.x+m_rect.width)return;
		if(y<m_rect.y || y>m_rect.y+m_rect.height)return;

		m_event = event;
	}
};

class UI:public BASE
{
public:
	UI();
	virtual ~UI();

	bool init(Config* pConfig, string* pName);
	bool draw(Frame* pFrame, iVec4* pTextPos);

	BUTTON* addBtn(BUTTON* pBtn);

	void handleMouse(int event, int x, int y, int flags);
	void handleKey(int key);

public:
	int 	m_nBtn;
	BUTTON	m_pBtn[NUM_BUTTON];


};

} /* namespace kai */

#endif /* SRC_UI_UI_H_ */
