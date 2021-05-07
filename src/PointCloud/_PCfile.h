/*
 * _PCfile.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCfile_H_
#define OpenKAI_src_PointCloud_PCfile_H_

#ifdef USE_OPEN3D
#include "_PCframe.h"

namespace kai
{

class _PCfile: public _PCframe
{
public:
	_PCfile();
	virtual ~_PCfile();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);

private:
	void update(void);
	static void* getUpdate(void* This)
	{
		((_PCfile *) This)->update();
		return NULL;
	}

public:
	string m_fName;
	PointCloud m_pc;

};

}
#endif
#endif