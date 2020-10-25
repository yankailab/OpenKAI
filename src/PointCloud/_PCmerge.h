/*
 * _PCmerge.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCmerge_H_
#define OpenKAI_src_PointCloud_PCmerge_H_

#include "../Base/common.h"

#ifdef USE_OPEN3D
#include "_PCbase.h"

namespace kai
{

class _PCmerge: public _PCbase
{
public:
	_PCmerge();
	virtual ~_PCmerge();

	bool init(void* pKiss);
	bool start(void);
	int check(void);

private:
	void updateMerge(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PCmerge *) This)->update();
		return NULL;
	}

public:
	vector<_PCbase*> m_vpPCB;

};

}
#endif
#endif
