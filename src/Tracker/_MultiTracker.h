/*
 * _MultiTracker.h
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__MultiTracker_H_
#define OpenKAI_src_Detector__MultiTracker_H_

#include "../Base/common.h"
#include "../Base/_ObjectBase.h"
#include "_SingleTracker.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

class _MultiTracker : public _ObjectBase
{
public:
	_MultiTracker();
	virtual ~_MultiTracker();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);
	bool cli(int& iY);

	bool addROI(vInt4 roi);

private:
	_SingleTracker* getTracker(void);
	void track(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_MultiTracker*) This)->update();
		return NULL;
	}

public:
	_SingleTracker*	m_pTracker;
	int				m_nTracker;

};

}
#endif
#endif
