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
		m_pPointCloud = (_PointCloud *)(pM->findModule(n));
#endif

		return true;
	}

	int _RGBDbase::getData(void *pOut, int iD, int nB)
	{
		NULL__(pOut, 0);

#ifdef USE_OPENCV
		if (iD == 0)
		{
			// RGB
			return this->_VisionBase::getData(pOut, iD, nB);
		}
		else if (iD == 1)
		{
			// Depth
			if (nB == 0)
				nB = m_mDepth.total() * m_mDepth.elemSize();

			memcpy(pOut, m_mDepth.data, nB);
		}
		else if (iD == 2)
		{
			// Transformed depth
			if (nB == 0)
				nB = m_mtDepth.total() * m_mtDepth.elemSize();

			memcpy(pOut, m_mtDepth.data, nB);
		}
		else if (iD == 3)
		{
			// Transformed RGB
			if (nB == 0)
				nB = m_mtRGB.total() * m_mtRGB.elemSize();

			memcpy(pOut, m_mtRGB.data, nB);
		}
		else if (iD == 4)
		{
			// IR
			if (nB == 0)
				nB = m_mIR.total() * m_mIR.elemSize();

			memcpy(pOut, m_mIR.data, nB);
		}

		return nB;
#endif

		return 0;
	}

	bool _RGBDbase::check(void)
	{
		return _VisionBase::check();
	}

	void _RGBDbase::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_VisionBase::console(pConsole);

		// _Console *pC = (_Console *)pConsole;
	}

#ifdef USE_OPENCV
	Mat *_RGBDbase::getDepthMat(void)
	{
		return &m_mDepth;
	}

	Vector2f _RGBDbase::getDepthRange(void)
	{
		return m_vRangeD;
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
