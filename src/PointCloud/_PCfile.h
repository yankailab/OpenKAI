/*
 * _PCfile.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCfile_H_
#define OpenKAI_src_PointCloud_PCfile_H_
#ifdef USE_OPEN3D

#include "_PCbase.h"

namespace kai
{

class _PCfile: public _PCbase
{
public:
	_PCfile();
	virtual ~_PCfile();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void draw(void);

public:
	string m_fName;

};

}
#endif
#endif
