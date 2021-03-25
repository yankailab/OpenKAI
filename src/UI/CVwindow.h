/*
 * Window.h
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_Window_H_
#define OpenKAI_src_UI_Window_H_
#ifdef USE_OPENCV
#include "../Base/BASE.h"
#include "../Vision/Frame.h"

namespace kai
{

class CVwindow: public BASE
{
public:
	CVwindow();
	virtual ~CVwindow();

	bool init(void *pKiss);
	void draw(void);
	Frame* getFrame(void);

	void addMsg(const string &pMsg, int iTab=0);
	double textSize(void);
	Scalar textColor(void);

public:
	bool m_bWindow;
	bool m_bFullScreen;
	bool m_bDrawMsg;

	Frame m_frame;
	vInt2 m_vSize;
	vInt2 m_vTextStart;
	int m_textY;
	int m_pixTab;
	int m_lineHeight;
	Point m_tPoint;
	float m_textSize;
	Scalar m_textCol;

	string m_gstOutput;
	VideoWriter m_gst;

	string m_fileRec;
	VideoWriter m_VW;

	Frame m_F;
	Frame m_F2;
};

}
#endif
#endif
