/*
#ifndef DETECTOR_DEPTH_H_
#define DETECTOR_DEPTH_H_

#include "../Base/common.h"
#include "../Stream/_StreamBase.h"
#include "../Navigation/_Universe.h"
#include "../Vision/_Flow.h"

namespace kai
{

class _Depth: public _ThreadBase
{
public:
	_Depth();
	~_Depth();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

private:
	void detect(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Depth*) This)->update();
		return NULL;
	}

public:
	_Universe* 		m_pUniverse;
	_StreamBase*	m_pStream;
	Frame*			m_pThrFrame;

	Mat m_Mat;

	double m_minArea;
	int m_threshold;
	int m_maxValue;
};
}

#endif
*/
