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
	m_nBtn = 0;

}

UI::~UI()
{
	// TODO Auto-generated destructor stub
}

bool UI::init(Config* pConfig)
{
	if (this->BASE::init(pConfig)==false)
		return false;

	//create buttons
	Config** pItr = pConfig->getChildItr();

	int i = 0;
	while (pItr[i])
	{
		Config* pBtn = pItr[i];
		i++;

		bool bInst = false;
		F_INFO(pBtn->v("bInst", &bInst));
		if (!bInst)continue;

		string className = "";
		F_INFO(pBtn->v("class", &className));
		if(className!="Btn")continue;

		BUTTON* pNewBtn = addBtn();
		NULL_F(pNewBtn);
		double r=0,g=0,b=0;

		F_ERROR_F(pBtn->v("name", &pNewBtn->m_name));
		CHECK_F(pNewBtn->m_name.empty());
		F_INFO(pBtn->v("x", &pNewBtn->m_rect.x));
		F_INFO(pBtn->v("y", &pNewBtn->m_rect.y));
		F_INFO(pBtn->v("w", &pNewBtn->m_rect.width));
		F_INFO(pBtn->v("h", &pNewBtn->m_rect.height));
		F_INFO(pBtn->v("r", &r));
		F_INFO(pBtn->v("g", &g));
		F_INFO(pBtn->v("b", &b));
		pNewBtn->m_color = Scalar(b,g,r);
		F_INFO(pBtn->v("tx", &pNewBtn->m_tx));
		F_INFO(pBtn->v("ty", &pNewBtn->m_ty));
		F_INFO(pBtn->v("fontSize", &pNewBtn->m_sizeFont));

	}

	pConfig->m_pInst = this;
	return true;
}

BUTTON* UI::addBtn(void)
{
	if(m_nBtn >= N_BUTTON)return NULL;

	BUTTON* pNew = &m_pBtn[m_nBtn];
	m_nBtn++;

	pNew->init();
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

BUTTON* UI::onMouse(MOUSE* pMouse)
{
	NULL_N(pMouse);
	BUTTON* pActive = NULL;

	int i;
	for(i=0; i<m_nBtn;i++)
	{
		if(m_pBtn[i].handler(pMouse->m_event,
							pMouse->m_x,
							pMouse->m_y,
							pMouse->m_flags))
		{
			pActive = &m_pBtn[i];
		}
	}

	return pActive;
}

void UI::onKey(int key)
{

}


} /* namespace kai */
