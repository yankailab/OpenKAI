/*
 * UI.h
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#ifndef SRC_UI_UI_H_
#define SRC_UI_UI_H_

#include "../Base/common.h"
#include "../Base/BASE.h"
#include "../Stream/Frame.h"


#define N_BUTTON 16

namespace kai
{

struct BUTTON
{
	string	m_name;
	Rect	m_rect;
	Scalar	m_color;
	int		m_tx;
	int		m_ty;
	double	m_sizeFont;
	int		m_event;

	void init()
	{
		m_name = "";
		m_rect.x = 0;
		m_rect.y = 0;
		m_rect.width = 0;
		m_rect.height = 0;
		m_color =Scalar(0,0,0);
		m_tx = 0;
		m_ty = 0;
		m_sizeFont = 0;
		m_event = 0;
	}

	bool handler(int event, int x, int y, int flags)
	{
		m_event = 0;
		if(x<m_rect.x || x>m_rect.x+m_rect.width)return false;
		if(y<m_rect.y || y>m_rect.y+m_rect.height)return false;

		m_event = event;
		return true;
	}
};

class UI:public BASE
{
public:
	UI();
	virtual ~UI();

	bool init(Config* pConfig);
	bool draw(Frame* pFrame, vInt4* pTextPos);

	BUTTON* addBtn(void);

	BUTTON* onMouse(MOUSE* pMouse);
	void onKey(int key);

public:
	int 	m_nBtn;
	BUTTON	m_pBtn[N_BUTTON];

};

} /* namespace kai */

#endif /* SRC_UI_UI_H_ */
