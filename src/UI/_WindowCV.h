/*
 * Window.h
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#ifndef OpenKAI_src_UI_WindowCV_H_
#define OpenKAI_src_UI_WindowCV_H_
#ifdef USE_OPENCV
#include "../Base/_ModuleBase.h"
#include "../Vision/Frame.h"

namespace kai
{

class _WindowCV: public _ModuleBase
{
public:
	_WindowCV();
	virtual ~_WindowCV();

	bool init(void *pKiss);
	bool start(void);

	Frame* getFrame(void);

protected:
	void updateWindow(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_WindowCV*) This)->update();
		return NULL;
	}

public:
	vector<BASE*> m_vpB;
	int	m_waitKey;

	bool m_bWindow;
	bool m_bFullScreen;

	Frame m_frame;
	vInt2 m_vSize;

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
