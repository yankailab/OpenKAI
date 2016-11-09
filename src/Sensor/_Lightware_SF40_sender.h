/*
 * _Lightware_SF40_sender_sender.h
 *
 *  Created on: Nov 4, 2016
 *      Author: yankai
 */

#ifndef AI__Lightware_SF40_sender_SENDER_H_
#define AI__Lightware_SF40_sender_SENDER_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Navigation/_Universe.h"
#include "../IO/SerialPort.h"
#include "../Algorithm/Filter.h"

#define DEG_AROUND 360.0

namespace kai
{

using std::string;
using namespace std;
using namespace cv;

class _Lightware_SF40_sender: public _ThreadBase
{
public:
	_Lightware_SF40_sender();
	~_Lightware_SF40_sender();

	bool init(Config* pConfig);
	bool link(void);
	void close(void);
	bool draw(Frame* pFrame, iVec4* pTextPos);

	std::vector<vector<float> > detect(Frame* pFrame);
	bool start(void);

private:
	void LD(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Lightware_SF40_sender*) This)->update();
		return NULL;
	}

public:
	SerialPort* m_pSerialPort;

	double	m_dAngle;

};

}

#endif
