/*
 * _Stream.h
 *
 *  Created on: Aug 23, 2015
 *      Author: yankai
 */

//#ifndef SRC_STREAM_STREAM_H_
//#define SRC_STREAM_STREAM_H_
/*

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Stream/Frame.h"
namespace kai
{

class _Stream: public _ThreadBase
{
public:
	_Stream();
	virtual ~_Stream();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	Frame* getBGRFrame(void);
	Frame* getGrayFrame(void);
	Frame* getHSVFrame(void);

public:
	bool		m_bHSV;
	bool		m_bGray;

private:
	_Camera*		m_pCamera;

	Frame*			m_pFrame;
	Frame*			m_pGrayFrame;
	Frame*			m_pHSVframe;

	int				m_showDepth;

private:
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Stream *) This)->update();
		return NULL;
	}
};

}

#endif */
