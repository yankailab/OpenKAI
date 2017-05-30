/*
 * _DNNGen_odometry.h
 *
 *  Created on: May 17, 2017
 *      Author: yankai
 */

#ifndef OPENKAI_SRC_Odometry__DNNGen_odometry_H_
#define OPENKAI_SRC_Odometry__DNNGen_odometry_H_

#include "../../../Base/common.h"
#include "../../../Base/_ThreadBase.h"
#include "../../../Vision/_ZED.h"
#include "../../../Vision/_Flow.h"
#include "../../../Utility/util.h"

namespace kai
{

class _DNNGen_odometry: public _ThreadBase
{
public:
	_DNNGen_odometry(void);
	virtual ~_DNNGen_odometry();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

private:
	void sample(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DNNGen_odometry *) This)->update();
		return NULL;
	}

public:
	_ZED* m_pZED;
	_Flow* m_pFlow;
	Frame* m_pFrameA;
	Frame* m_pFrameB;

	int	m_zedMinConfidence;
	int	m_uDelay;
	double	m_dMinTot;

	int			m_nGen;
	int			m_iGen;
	string 		m_outDir;
	ofstream	m_ofs;

	bool		m_bCrop;
	Rect		m_cropBB;

};

}

#endif
