/*
 * _FastLivo2.cpp
 *
 *  Created on: Nov 12, 2024
 *      Author: yankai
 */

#include "_FastLivo2.h"

namespace kai
{

	_FastLivo2::_FastLivo2()
	{
		m_bImg = false;
		m_vSizeImg.set(640, 480);

		m_vF.set(520, 520);
		m_vC.set(320, 240);
	}

	_FastLivo2::~_FastLivo2()
	{
	}

	bool _FastLivo2::init(const json &j)
	{
		IF_F(!this->_SLAMbase::init(j));

		jKv(j, "bImg", m_bImg);
		jKv<int>(j, "vSizeImg", m_vSizeImg);

		jKv(j, "nFdownSample", m_config.m_nFdownSample);
		jKv(j, "nPmax", m_config.m_nPmax);
		jKv(j, "voxelLeafSize", m_config.m_voxelLeafSize);

		jKv<double>(j, "vF", m_vF);
		jKv<double>(j, "vC", m_vC);
		jKv(j, "aILr", m_aILr);
		jKv(j, "aILt", m_aILt);
		jKv(j, "aCLr", m_aCLr);
		jKv(j, "aCLt", m_aCLt);

		return setup();
	}

	bool _FastLivo2::setup(void)
	{
		IF_F(!m_fastLivo.init(""));

		m_fastLivo.setImageEnabled(m_bImg);

		IF_F(!m_fastLivo.setCameraPinholeDistorted(m_vSizeImg.x, m_vSizeImg.y, m_vF.x, m_vF.y, m_vC.x, m_vC.y, m_aDist));
		IF_F(!m_fastLivo.setImuToLidarExtrinsic(m_aILr, m_aILt));
		IF_F(!m_fastLivo.setLidarToCameraExtrinsic(m_aCLr, m_aCLt));

		IF_F(!m_fastLivo.configureColoredMapMaintenance(m_config.m_nFdownSample, m_config.m_nPmax, m_config.m_voxelLeafSize));

		return true;
	}

	void _FastLivo2::reset(void)
	{

	}

	bool _FastLivo2::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_SLAMbase::link(j, pM));

		return true;
	}

	bool _FastLivo2::check(void)
	{
		return this->_SLAMbase::check();
	}

	bool _FastLivo2::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _FastLivo2::update(void)
	{
		while (m_pT->bAlive())
		{
//			m_pT->autoFPS();

			m_pT->sleepT(0);

			updateFastLivo2();
		}
	}

	void _FastLivo2::updateFastLivo2(void)
	{
		IF_(!check());

		// IMU
		Vector3d vG, vA;
		uint64_t tIMU;
		while ((tIMU = this->getIMUpair(&vG, &vA)) > 0)
		{
			ImuSample imu;
			imu.t = tIMU;
			imu.acc = vA;
			imu.gyro = vG;
			m_fastLivo.pushImu(imu);
		}

		// point cloud
		m_fastLivo.pushLidar(m_LS);

		// image
		if (m_bImg)
		{
			ImageFrame fImg;
			m_fastLivo.pushImage(fImg);
		}

		IF_(!m_fastLivo.spinOnce());

		Pose pFL;
		m_vT.x = pFL.p[0];
		m_vT.y = pFL.p[1];
		m_vT.z = pFL.p[2];
	}

	ImageFrame _FastLivo2::makeTestBGR(double t, int w, int h)
	{
		ImageFrame f;
		f.t = t;
		f.width = w;
		f.height = h;
		f.bgr.resize((size_t)w * (size_t)h * 3);

		// Simple moving gradient (BGR)
		for (int y = 0; y < h; ++y)
		{
			for (int x = 0; x < w; ++x)
			{
				const int idx = (y * w + x) * 3;
				f.bgr[idx + 0] = (uint8_t)((x + (int)(t * 50)) % 256);		 // B
				f.bgr[idx + 1] = (uint8_t)((y + (int)(t * 30)) % 256);		 // G
				f.bgr[idx + 2] = (uint8_t)(((x + y) + (int)(t * 10)) % 256); // R
			}
		}
		return f;
	}

	LidarScan _FastLivo2::makeTestScan(double t0, double t1, int npts)
	{
		LidarScan s;
		s.t0 = t0;
		s.t1 = t1;
		s.pts.resize((size_t)npts);

		// simple rotating ring in lidar frame
		for (int i = 0; i < npts; ++i)
		{
			const double a = (2.0 * M_PI) * (double(i) / double(npts));
			const double r = 8.0 + 0.5 * std::sin(3.0 * a);
			LidarPoint p;
			p.x = float(r * std::cos(a));
			p.y = float(r * std::sin(a));
			p.z = float(0.5 * std::sin(a * 2.0));
			p.intensity = float((i % 100) / 100.0);
			p.t = t0 + (t1 - t0) * (double(i) / double(npts)); // per-point time
			s.pts[i] = p;
		}
		return s;
	}

	void _FastLivo2::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_SLAMbase::console(pConsole);

		_Console *pC = (_Console *)pConsole;
	}

}
