/*
 * _PCfile.cpp
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#include "_PCfile.h"

#ifdef USE_OPEN3D

namespace kai
{

_PCfile::_PCfile()
{
}

_PCfile::~_PCfile()
{
}

bool _PCfile::init(void *pKiss)
{
	IF_F(!_PCbase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("fName", &m_fName);
	open();

	return true;
}

bool _PCfile::open(void)
{
	IF_F(m_fName.empty());

//	io::ReadPointCloudOption ro;
	PointCloud pc;
	IF_F(!io::ReadPointCloud(m_fName, pc));

	for(int i=0; i<pc.points_.size(); i++)
	{
		add(pc.points_[i], pc.colors_[i]);
	}

	return true;
}

bool _PCfile::start(void)
{
	return true;
}

void _PCfile::draw(void)
{
	this->_ModuleBase::draw();
}

}
#endif
