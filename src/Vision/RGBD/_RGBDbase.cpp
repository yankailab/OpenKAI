/*
 * _RGBDbase.cpp
 *
 *  Created on: Jan 2, 2024
 *      Author: yankai
 */

#include "_RGBDbase.h"

namespace kai
{

	_RGBDbase::_RGBDbase()
	{
	}

	_RGBDbase::~_RGBDbase()
	{
		DEL(m_pTpp);
	}

	bool _RGBDbase::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		jKv(j, "devFPSd", m_devFPSd);
		jKv<int>(j, "vSizeD", m_vSizeD);
		jKv<float>(j, "vRangeD", m_vRangeD);
		jKv(j, "dScale", m_dScale);
		jKv(j, "dOfs", m_dOfs);

		jKv(j, "bDepth", m_bDepth);
		jKv(j, "bIR", m_bIR);
		jKv(j, "btRGB", m_btRGB);
		jKv(j, "btDepth", m_btDepth);
		jKv(j, "bConfidence", m_bConfidence);
		jKv(j, "fConfidenceThr", m_fConfidenceThr);

		jKv(j, "bIMU", m_bIMU);
		jKv(j, "bPCL", m_bPCL);
		jKv(j, "bPCLrgb", m_bPCLrgb);

		return true;
	}

	bool _RGBDbase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n;

		n = "";
		jKv(j, "_IMUbase", n);
		m_pIMU = (_IMUbase *)(pM->findModule(n));

#ifdef WITH_UNIVERSE
		n = "";
		jKv(j, "_PointCloud", n);
		m_pPCL = (_PointCloud *)(pM->findModule(n));
#endif

		return true;
	}

	bool _RGBDbase::check(void)
	{
		return _VisionBase::check();
	}

	Vector2f _RGBDbase::getDepthRange(void)
	{
		return m_vRangeD;
	}

	void _RGBDbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_VisionBase::console(pConsole);

		// _Console *pC = (_Console *)pConsole;
	}

#ifdef USE_OPENCV
	Mat *_RGBDbase::getMatDepth(void)
	{
		return &m_mDepth;
	}

	Mat *_RGBDbase::getMatTransformedDepth(void)
	{
		return &m_mtDepth;
	}

	Mat *_RGBDbase::getMatTransformedRGB(void)
	{
		return &m_mtRGB;
	}

	Mat *_RGBDbase::getMatIR(void)
	{
		return &m_mIR;
	}

	void _RGBDbase::draw(void *pMat)
	{
		NULL_(pMat);
		this->_VisionBase::draw(pMat);
		IF_(!check());
		IF_(m_mRGB.empty());

	}
#endif

}
