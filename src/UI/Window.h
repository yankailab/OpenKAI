/*
 * Window.h
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_UI_WINDOW_H_
#define OPENKAI_SRC_UI_WINDOW_H_

#include "../Base/common.h"
#include "../Base/BASE.h"
#include "../Stream/Frame.h"

#define TAB_PIX 25
#define LINE_HEIGHT 15;

namespace kai
{

class Window: public BASE
{
public:
	Window();
	virtual ~Window();

	bool init(void* pKiss);
	bool link(void);
	bool draw(void);
	Frame* getFrame(void);
	Point* getTextPos(void);

	void addMsg(string* pMsg);

	void tabNext(void);
	void tabPrev(void);
	void tabReset(void);

	void lineNext(void);
	void lineReset(void);

	double textSize(void);
	Scalar textColor(void);

public:
	bool	m_bWindow;
	bool	m_bFullScreen;
	bool	m_bRec;
	Frame*	m_pFrame;
	vInt2	m_size;
	vInt2	m_textPos;
	vInt2	m_textStart;
	int		m_pixTab;
	int		m_lineHeight;
	Point	m_tPoint;
	double	m_textSize;
	Scalar	m_textCol;

	VideoWriter m_VW;
};

}

#endif
