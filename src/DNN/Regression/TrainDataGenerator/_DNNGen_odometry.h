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
	_ZED*	m_pZED;
	Frame*	m_pPrev;
	Frame*	m_pNext;
	Frame*	m_pDepth;
	bool	m_bCount;

	int	m_zedMinConfidence;
	double	m_dMinTot;
	int m_interval;

	int			m_nGen;
	int			m_iGen;
	string 		m_outDir;
	ofstream	m_ofs;
	string		m_format;
	string		m_fNamePrefix;
	string		m_fNameList;

	bool		m_bResize;
	int			m_width;
	int			m_height;
	bool		m_bCrop;
	Rect		m_cropBB;

};

}

#endif
