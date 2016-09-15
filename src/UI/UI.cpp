/*
 * UI.cpp
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#include "UI.h"

namespace kai
{

UI::UI()
{

}

UI::~UI()
{
	// TODO Auto-generated destructor stub
}

bool UI::init(Config* pConfig, string name)
{
	if(pConfig==NULL)return false;
	if(name.empty())return false;


	return true;
}

BUTTON* UI::addBtn(BUTTON* pBtn)
{
	if(pBtn==NULL)return NULL;
	if(m_nBtn >= NUM_BUTTON)return NULL;

	BUTTON* pNew = &m_pBtn[m_nBtn];
	m_nBtn++;

	*pNew = *pBtn;
	return pNew;
}

bool UI::draw(Frame* pFrame, iVec4* pTextPos)
{
	if (pFrame == NULL)
		return false;

	Mat* pMat = pFrame->getCMat();

	int i;
	for(i=0; i<m_nBtn;i++)
	{
		BUTTON* pBtn = &m_pBtn[i];
		putText(*pMat, pBtn->m_name, cv::Point(pBtn->m_rect.x,pBtn->m_rect.y),
				FONT_HERSHEY_SIMPLEX, pBtn->m_sizeFont, pBtn->m_color, 1);
		rectangle( *pMat, pBtn->m_rect, pBtn->m_color, 1 );
	}

	return true;
}

void UI::handleMouse(int event, int x, int y, int flags)
{
	int i;
	for(i=0; i<m_nBtn;i++)
	{
		m_pBtn[i].handler(event,x,y,flags);
	}
}

void UI::handleKey(int key)
{

}


} /* namespace kai */
