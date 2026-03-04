/*
 * _Orbbec.cpp
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#include "_Orbbec.h"

namespace kai
{

	_Orbbec::_Orbbec()
	{
		m_SN = "";
		m_tOutMs = 100;
	}

	_Orbbec::~_Orbbec()
	{
		close();
	}

	bool _Orbbec::init(const json &j)
	{
		IF_F(!_RGBDbase::init(j));

		jKv(j, "SN", m_SN);
		jKv(j, "tOutMs", m_tOutMs);

		DEL(m_pTpp);
		m_pTpp = createThread(jK(j, "threadPP"), "threadPP");
		NULL_F(m_pTpp);

		return true;
	}

	bool _Orbbec::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_RGBDbase::link(j, pM));

		return true;
	}

	bool _Orbbec::open(void)
	{
		IF__(m_bOpen, true);

		shared_ptr<ob::DeviceList> pDL = m_ctx.queryDeviceList();
		if (m_SN.empty())
		{
			m_spDev = pDL->getDevice(0);
		}
		else
		{
			m_spDev = pDL->getDeviceBySN(m_SN.c_str());
		}

		// Hardware noise removal filter.
		if (m_spDev->isPropertySupported(OB_PROP_HW_NOISE_REMOVE_FILTER_ENABLE_BOOL, OB_PERMISSION_READ_WRITE))
		{
			m_spDev->setBoolProperty(OB_PROP_HW_NOISE_REMOVE_FILTER_ENABLE_BOOL, true);
			m_spDev->setFloatProperty(OB_PROP_HW_NOISE_REMOVE_FILTER_THRESHOLD_FLOAT, 0.1);
		}

		m_spSensorList = m_spDev->getSensorList();
		for (int i = 0; i < m_spSensorList->getCount(); i++)
		{
			OBSensorType sensorType = m_spSensorList->getSensorType(i);
			LOG_I("Supported Sensor type: " + i2str(sensorType));
		}

		// Pipeline
		m_spPipe = make_shared<ob::Pipeline>(m_spDev);
		m_spConfig = make_shared<ob::Config>();

		// RGBD streams
		if (m_bRGB)
			m_spConfig->enableVideoStream(OB_STREAM_COLOR, m_vSizeRGB.x, m_vSizeRGB.y, m_devFPS, OB_FORMAT_BGR);
		if (m_bDepth)
			m_spConfig->enableVideoStream(OB_STREAM_DEPTH, m_vSizeD.x, m_vSizeD.y, m_devFPSd, OB_FORMAT_Y16);

		// IMU streams
		if (m_bIMU)
		{
			m_spConfig->enableAccelStream(OB_ACCEL_FS_4g, OB_SAMPLE_RATE_200_HZ);
			m_spConfig->enableGyroStream(OB_GYRO_FS_1000dps, OB_SAMPLE_RATE_200_HZ);
		}

		// For point cloud generation
		if (m_bPCrgb)
		{
			m_spConfig->setFrameAggregateOutputMode(OB_FRAME_AGGREGATE_OUTPUT_ALL_TYPE_FRAME_REQUIRE);
			m_spConfig->setAlignMode(ALIGN_D2C_HW_MODE);
			m_spPipe->enableFrameSync();
		}

		m_spPipe->start(m_spConfig);

		// Point cloud filter
		m_spPCF = std::make_shared<ob::PointCloudFilter>();
		//		m_spPCF->setCameraParam(m_spPipe->getCameraParam());
		if (m_bPCrgb)
		{
			m_spPCF->setCreatePointFormat(OB_FORMAT_RGB_POINT);
		}
		else if (m_bPCd)
		{
			m_spPCF->setCreatePointFormat(OB_FORMAT_POINT);
		}

		m_bOpen = true;
		return true;
	}

	void _Orbbec::close(void)
	{
		IF_(!m_bOpen);

		m_spPipe->stop();
		m_bOpen = false;
	}

	bool _Orbbec::start(void)
	{
		NULL_F(m_pT);
		NULL_F(m_pTpp);

		IF_F(!m_pT->start(getUpdate, this));
		return m_pTpp->start(getTPP, this);
	}

	bool _Orbbec::check(void)
	{
		return this->_RGBDbase::check();
	}

	void _Orbbec::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					LOG_E("Cannot open Orbbec");
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPS();

			if (updateOrbbec())
			{
				m_pTpp->run();
			}
			else
			{
				close();
			}
		}
	}

	bool _Orbbec::updateOrbbec(void)
	{
		IF_F(!check());

		shared_ptr<ob::FrameSet> spFS = m_spPipe->waitForFrameset(m_tOutMs);
		NULL_F(spFS);

		// IMU, fast stream
		if (auto g = spFS->getFrame(OB_FRAME_GYRO))
		{
			auto gf = g->as<ob::GyroFrame>();
			if (gf)
			{
				auto v = gf->value();
				m_IMUpair.pushGyro(frameTsUs_(g), {v.x, v.y, v.z});
			}
		}

		if (auto a = spFS->getFrame(OB_FRAME_ACCEL))
		{
			auto af = a->as<ob::AccelFrame>();
			if (af)
			{
				auto v = af->value();
				m_IMUpair.pushAcc(frameTsUs_(a), {v.x, v.y, v.z});
			}
		}
		// Emit paired IMU samples into core (fast)
		//		m_IMUpair.flushToCore(core_);

		// Images, slow stream
		shared_ptr<ob::Frame> spFrameRGB = nullptr;
		shared_ptr<ob::Frame> spFrameD = nullptr;

		if (m_bRGB)
		{
			spFrameRGB = spFS->getFrame(OB_FRAME_COLOR);
			if (spFrameRGB)
			{
				*m_fRGB.m() = Mat(m_vSizeRGB.y, m_vSizeRGB.x, CV_8UC3, spFrameRGB->getData());
			}
		}

		if (m_bDepth)
		{
			spFrameD = spFS->getFrame(OB_FRAME_DEPTH);
			if (spFrameD)
			{
				*m_fDepth.m() = Mat(m_vSizeD.y, m_vSizeD.x, CV_16UC1, spFrameD->getData());
			}
		}

		// Point cloud
		if (m_bPCrgb && spFrameRGB && spFrameD)
		{
			m_spFrame = m_spPCF->process(spFS);
		}
		else if (m_bPCd && spFrameD)
		{
			m_spFrame = m_spPCF->process(spFS);
		}

		return true;
	}

	void _Orbbec::updateTPP(void)
	{
		while (m_pTpp->bAlive())
		{
			m_pTpp->sleepT(0);

#ifdef USE_OPENCV
			// Mat mZ = Mat(Size(m_vSizeD.x, m_vSizeD.y), CV_16UC1, (void *)m_rsDepth.get_data(), Mat::AUTO_STEP);
			// Mat mD, mDs;
			// mZ.convertTo(mD, CV_32FC1);
			// mDs = mD * m_dScale;
			// m_fDepth.copy(mDs + m_dOfs);

#endif

			//            updatePointCloud();
		}
	}

#ifdef WITH_3D
	int _Orbbec::getPointCloud(_PCframe *pPCframe, int nPmax)
	{
		NULL__(m_spFrame, -1);
		NULL__(pPCframe, -1);
		PointCloud *pPC = pPCframe->getNextBuffer();

		const int n = int(m_spFrame->dataSize() / sizeof(OBPoint));
		const OBPoint *pts = (const OBPoint *)m_spFrame->data();

		const float s_b = 1.0 / 1000.0;
		const float c_b = 1.0 / 255.0;

		int nPi = 0;
		for (int i = 0; i < n; i++)
		{
			const auto &p = pts[i];
			IF_CONT(!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z));
			IF_CONT(p.z <= 0);

			Eigen::Vector3d vP(p.x, p.y, p.z);
			vP *= s_b;
			IF_CONT(vP.z() < m_vRangeD.x);
			IF_CONT(vP.z() > m_vRangeD.y);

			pPC->points_.push_back(vP);

			// texture color
			//			Eigen::Vector3d vC(pC[2], pC[1], pC[0]);
			Eigen::Vector3d vC(255, 255, 255);
			vC *= c_b;
			pPC->colors_.push_back(vC);

			nPi++;
			IF__(nPi >= nPmax, nPi);
		}

		return nPi;
	}
#endif

	void _Orbbec::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_RGBDbase::console(pConsole);

		_Console *pC = (_Console *)pConsole;

		int nD = 3;
		TimedVec3 vGyro = m_IMUpair.getGyro();
		pC->addMsg("vGyro = (" + lf2str(vGyro.v[0], nD) + ", " + lf2str(vGyro.v[1], nD) + ", " + lf2str(vGyro.v[2], nD) + "), t=" + li2str(vGyro.t_us));

		TimedVec3 vAcc = m_IMUpair.getAcc();
		pC->addMsg("vAcc  = (" + lf2str(vAcc.v[0], nD) + ", " + lf2str(vAcc.v[1], nD) + ", " + lf2str(vAcc.v[2], nD) + "), t=" + li2str(vAcc.t_us));
	}

}
