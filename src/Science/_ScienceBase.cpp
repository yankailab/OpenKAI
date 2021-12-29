/*
 * _ScienceBase.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_ScienceBase.h"

namespace kai
{

	_ScienceBase::_ScienceBase()
	{
	}

	_ScienceBase::~_ScienceBase()
	{
	}

	bool _ScienceBase::init(void *pKiss)
	{
		IF_F(!this->_ModuleBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		return true;
	}

	int _ScienceBase::cbGraph(mglGraph *pGr)
	{
		pGr->Title("Graph");
		pGr->SetOrigin(0, 0);
		pGr->SetRanges(0, 10, -2.5, 2.5);
		pGr->FPlot("sin(1.7*2*pi*x) + sin(1.9*2*pi*x)", "r-4");
		pGr->Axis();
		pGr->Grid();
		return 0;
	}

	void _ScienceBase::cvDraw(void *pWindow)
	{
	}

}
