/*
 * CamMonitor.h
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

#ifndef SRC_CAMMONITOR_H_
#define SRC_CAMMONITOR_H_

#include "../Base/common.h"
#include "../Base/cvplatform.h"
#include "../Stream/_Stream.h"

#define NUM_MONITOR_FRAME 8

namespace kai {

struct MONITOR_FRAME {
	Frame* m_pFrame;
	string m_name;
	int m_x;
	int m_y;
	int m_w;
	int m_h;
};

class UIMonitor {
public:
	UIMonitor();
	virtual ~UIMonitor();

	bool init(string name, JSON* pJson);
	bool addFullFrame(Frame* pFrame);
	bool addFrame(Frame* pFrame, int x, int y, int w, int h);
	bool addFrame(MONITOR_FRAME* pMFrame);
	bool updateFrame(MONITOR_FRAME* pMFrame);
	void removeFrame(Frame* pFrame);
	void removeAll(void);
	void show(void);

public:
	string m_windowName;
	int m_width;
	int m_height;
	Mat m_showFrame;
	Mat m_frame;

	Frame m_camFrame;
	Frame m_camFrame2;
	MONITOR_FRAME m_pFrame[NUM_MONITOR_FRAME];
	int m_numFrame;

};

} /* namespace kai */

#endif /* SRC_CAMMONITOR_H_ */
