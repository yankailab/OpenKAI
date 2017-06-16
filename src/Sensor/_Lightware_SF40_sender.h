/*
 * _Lightware_SF40_sender_sender.h
 *
 *  Created on: Nov 4, 2016
 *      Author: yankai
 */

#ifndef SENSOR__Lightware_SF40_SENDER_H_
#define SENSOR__Lightware_SF40_SENDER_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../IO/_SerialPort.h"
#include "../Vision/Frame.h"

#define DEG_AROUND 360.0

using std::string;
using namespace std;

namespace kai
{

class _Lightware_SF40_sender: public _ThreadBase
{
public:
	_Lightware_SF40_sender();
	~_Lightware_SF40_sender();

	bool init(void* pKiss);
	bool link(void);
	void close(void);
	bool draw(void);

	std::vector<vector<float> > detect(Frame* pFrame);
	bool start(void);

	void MBS(uint8_t MBS);

private:
	void LD(void);
	void Map(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Lightware_SF40_sender*) This)->update();
		return NULL;
	}

public:
	_SerialPort* m_pSerialPort;
	double	m_dAngle;

};

}

#endif
