/*
 * _RGBDpointCloudBase.cpp
 *
 *  Created on: Jan 8, 2024
 *      Author: yankai
 */

#include "_RGBDpointCloudBase.h"

namespace kai
{

    _RGBDpointCloudBase::_RGBDpointCloudBase()
    {
    }

    _RGBDpointCloudBase::~_RGBDpointCloudBase()
    {
    }

    bool _RGBDpointCloudBase::init(void *pKiss)
    {
        IF_F(!_RGBDbase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

//        pK->v("vSizeD", &m_vSizeD);

        return true;
    }

    bool _RGBDpointCloudBase::link(void)
    {
        IF_F(!this->_RGBDbase::link());
        Kiss *pK = (Kiss *)m_pKiss;

        string n;

        // n = "";
        // pK->v("_SHMtransformedRGB", &n);
        // m_psmTransformedRGB = (_SharedMem *)(pK->getInst(n));

        return true;
    }

    int _RGBDpointCloudBase::check(void)
    {
        return _RGBDbase::check();
    }

	void _RGBDpointCloudBase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_RGBDbase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
//		pC->addMsg("", 0);
	}

}
