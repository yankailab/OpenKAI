/*
 * _Lightware_SF40.h
 *
 *  Created on: Nov 4, 2016
 *      Author: yankai
 */

#ifndef AI__Lightware_SF40_H_
#define AI__Lightware_SF40_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Navigation/_Universe.h"
#include "../IO/SerialPort.h"


namespace kai
{

using std::string;
using namespace std;
using namespace cv;

class _Lightware_SF40: public _ThreadBase
{
public:
	_Lightware_SF40();
	~_Lightware_SF40();

	bool init(Config* pConfig);
	bool link(void);
	void close(void);
	bool draw(Frame* pFrame, iVec4* pTextPos);

	std::vector<vector<float> > detect(Frame* pFrame);
	bool start(void);

private:
	void updateLidar(void);
	void write(void);
	void read(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Lightware_SF40*) This)->update();
		return NULL;
	}

private:

public:
	_Universe* m_pUniverse;
	string m_sportName;
	SerialPort* m_pSerialPort;
	int m_baudRate;

};

}

#endif
