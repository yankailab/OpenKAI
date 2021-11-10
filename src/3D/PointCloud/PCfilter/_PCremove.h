/*
 * _PCremove.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_PCremove_H_
#define OpenKAI_src_3D_PointCloud_PCremove_H_

#include "../../_GeometryBase.h"

namespace kai
{

class _PCremove: public _GeometryBase
{
public:
	_PCremove();
	virtual ~_PCremove();

	bool init(void* pKiss);
	bool start(void);
	int check(void);

private:
	void update(void);
	void updateFilter(void);
	static void* getUpdate(void* This)
	{
		(( _PCremove *) This)->update();
		return NULL;
	}

public:
    int m_nP;
    double m_r;

};

}
#endif
