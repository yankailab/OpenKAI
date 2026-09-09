/*
 * _GeometryViewerBase.cpp
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#include "_GeometryViewerBase.h"

namespace kai
{

	_GeometryViewerBase::_GeometryViewerBase()
	{
		m_vWinSize.set(1280, 720);
		m_bFullScreen = false;
		m_dirSave = "";
		m_vCoR.set(0, 0, 0);

		m_dTexpire = 0;
		m_nPbuf = 200000;
		m_nLbuf = 100000;
	}

	_GeometryViewerBase::~_GeometryViewerBase()
	{
		m_grPt.release();
		m_grLn.release();
	}

	bool _GeometryViewerBase::init(const json &j)
	{
		IF_F(!this->_GeometryBase::init(j));

		jKv<int>(j, "vWinSize", m_vWinSize);
		jKv(j, "bFullScreen", m_bFullScreen);
		jKv(j, "dirSave", m_dirSave);

		jKv(j, "camProjType", m_camProj.m_type);
		jKv(j, "camFov", m_camProj.m_fov);
		jKv<float>(j, "vCamLR", m_camProj.m_vLR);
		jKv<float>(j, "vCamBT", m_camProj.m_vBT);
		jKv<float>(j, "vCamNF", m_camProj.m_vNF);

		jKv<float>(j, "vCamLookAt", m_camPose.m_vLookAt);
		jKv<float>(j, "vCamEye", m_camPose.m_vEye);
		jKv<float>(j, "vCamUp", m_camPose.m_vUp);
		m_camPoseDefault = m_camPose;

		jKv<float>(j, "vCoR", m_vCoR);
		jKv(j, "dTexpire", m_dTexpire);

		jKv(j, "nPbuf", m_nPbuf);
		jKv(j, "nLbuf", m_nLbuf);

		m_grPt.release();
		m_grLn.release();
		IF_Le_F(m_nPbuf < 0 || m_nLbuf < 0, "Negative geometry buffer limit");
		IF_Le_F(m_nPbuf && !m_grPt.alloc(m_nPbuf), "Alloc failed with nPbuf: " + i2str(m_nPbuf));
		IF_Le_F(m_nLbuf && !m_grLn.alloc(m_nLbuf), "Alloc failed with nLbuf: " + i2str(m_nLbuf));

		return true;
	}

	bool _GeometryViewerBase::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_GeometryBase::link(j, pM));

		vector<string> vGb;
		jKv(j, "vGeometryBase", vGb);
		m_vpGb.clear();
		for (string n : vGb)
		{
			_GeometryBase *pGB = (_GeometryBase *)(pM->findModule(n));
			IF_CONT(!pGB);

			m_vpGb.push_back(pGB);
		}

		return true;
	}

	bool _GeometryViewerBase::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->startThread(getUpdate, this));

		return true;
	}

	bool _GeometryViewerBase::check(void)
	{
		return this->_GeometryBase::check();
	}

	void _GeometryViewerBase::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			updateAllGeometries();

			// override this method in inherited class
			// update front end in the inherited class
		}
	}

	void _GeometryViewerBase::updateAllGeometries(void)
	{
		IF_(!check());

		// override this method in inherited class

		uint64_t tExpire = 0;
		if(m_dTexpire > 0)
			tExpire = getApproxTbootUs() - m_dTexpire;

		for (_GeometryBase *pGb : m_vpGb)
		{
			m_grPt.clear();
			pGb->get(&m_grPt, tExpire);
			// update the point cloud buffer to frontend ingerited class

			m_grLn.clear();
			pGb->get(&m_grLn, tExpire);
			// update the line buffer to frontend ingerited class
		}
	}

	void _GeometryViewerBase::resetCamPose(void)
	{
		m_camPose = m_camPoseDefault;
	}

	void _GeometryViewerBase::setCamPose(const GVIEWER_CAM_POSE &camPose)
	{
		m_camPose = camPose;
		updateCamPose();
	}

	GVIEWER_CAM_POSE _GeometryViewerBase::getCamPose(void)
	{
		return m_camPose;
	}

	void _GeometryViewerBase::setCamProj(const GVIEWER_CAM_PROJ &camProj)
	{
		m_camProj = camProj;
		updateCamProj();
	}

	GVIEWER_CAM_PROJ _GeometryViewerBase::getCamProj(void)
	{
		return m_camProj;
	}

	void _GeometryViewerBase::updateCamProj(void)
	{
		IF_(!check());

		// override this method in inherited class
		// update camera projection parameters by m_camProj
	}

	void _GeometryViewerBase::updateCamPose(void)
	{
		IF_(!check());

		// override this method in inherited class
		// update camera pose parameters by m_camProj
	}

}
