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
		m_vWinSize = Vector2i(1280, 720);
		m_vCoR = Vector3f(0, 0, 0);
	}

	_GeometryViewerBase::~_GeometryViewerBase()
	{
		m_grPt.release();
		m_grLn.release();
	}

	bool _GeometryViewerBase::loadConfig(void)
	{
		IF_F(!this->_GeometryBase::loadConfig());
		const json &j = *m_pJ;

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

		if (!m_bGeometryBuffers)
		{
			m_nPbuf = 0;
			m_nLbuf = 0;
		}

		m_grPt.release();
		m_grLn.release();
		IF_Le_F(m_nPbuf < 0 || m_nLbuf < 0, "Negative geometry buffer limit");
		IF_Le_F(m_nPbuf && !m_grPt.alloc(m_nPbuf), "Alloc failed with nPbuf: " + i2str(m_nPbuf));
		IF_Le_F(m_nLbuf && !m_grLn.alloc(m_nLbuf), "Alloc failed with nLbuf: " + i2str(m_nLbuf));

		return true;
	}

	bool _GeometryViewerBase::link(void)
	{
		NULL_F(m_pM);
		IF_F(!this->_GeometryBase::link());

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
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			updateAllGeometries();

			// override this method in inherited class
			// update front end in the inherited class
		}
	}

	void _GeometryViewerBase::updateAllGeometries(void)
	{
		// Concrete viewers own and collect their typed sources.
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
