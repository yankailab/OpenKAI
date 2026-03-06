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
		m_pPCin = nullptr;
		m_pPCout = nullptr;
		m_iP = 0;
		m_nPmax = 100000;
		m_tStampP = 1;

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

		jKv(j, "nPmax", m_nPmax);

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

		string n;

		n = "";
		jKv(j, "_PCin", n);
		m_pPCin = (_PCstream *)(pM->findModule(n));

		n = "";
		jKv(j, "_PCout", n);
		m_pPCout = (_PCstream *)(pM->findModule(n));

		return true;
	}

	bool _FastLivo2::check(void)
	{
		NULL_F(m_pPCin);
		NULL_F(m_pIMU);

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
			m_pT->autoFPS();

			if (updateFastLivo2())
			{
				updatePointCloud();
			}
		}
	}

	bool _FastLivo2::updateFastLivo2(void)
	{
		IF_F(!check());

		// IMU
		vFloat3 vG, vA;
		uint64_t tIMU;
		while ((tIMU = m_pIMU->getIMUpair(&vG, &vA)) > 0)
		{
			ImuSample imu;
			imu.t = (double)tIMU * m_tScaleIMU;
			imu.acc = v2e(vA).cast<double>();
			imu.gyro = v2e(vG).cast<double>();
			m_fastLivo.pushImu(imu);
		}

		// point cloud
		LidarScan LS;
		int nPring = m_pPCin->nP();
		int nP = 0;
		GEOMETRY_POINT *pGp;

		while (pGp = m_pPCin->get(m_iP))
		{
			if (!pGp)
				break;
			if (pGp->m_tStamp < m_tStampP)
				break;
			if (nP >= m_nPmax)
				break;

			LidarPoint lp;
			lp.x = pGp->m_vP.x;
			lp.y = pGp->m_vP.y;
			lp.z = pGp->m_vP.z;
			lp.intensity = 1.0f;
			lp.tag = 0;
			lp.t = (double)pGp->m_tStamp * m_tScalePC;
			LS.pts.push_back(lp);

			m_tStampP = pGp->m_tStamp;
			m_iP = iRing(m_iP, nPring);
			nP++;
		}

		IF_F(nP <= 0);
		LS.t0 = LS.pts.front().t;
		LS.t1 = LS.pts.back().t; // scan.t0 + dTs;
								 //		scan.pts.reserve(n);
								 //		double dTp = dTs / n;
		m_fastLivo.pushLidar(LS);

		// image
		if (m_bImg)
		{
			ImageFrame fImg;
			m_fastLivo.pushImage(fImg);
		}

		IF_F(!m_fastLivo.spinOnce());

		Pose pFL;
		m_fastLivo.getPose(pFL);
		m_vT.x = pFL.p[0];
		m_vT.y = pFL.p[1];
		m_vT.z = pFL.p[2];

		return true;
	}

	void _FastLivo2::updatePointCloud(void)
	{
		NULL_(m_pPCout);

		pcl::PointCloud<pcl::PointXYZI>::Ptr pPC = m_fastLivo.getMap();
		NULL_(pPC);
		IF_(pPC->points.empty());

		const pcl::PointXYZI *ptr = pPC->points.data();
		const size_t n = pPC->points.size();
		for (size_t i = 0; i < n; ++i)
		{
			const pcl::PointXYZI &p = ptr[i];
			vFloat3 vP(p.x, p.y, p.z);
			vFloat3 vC(p.intensity);
			m_pPCout->add(vP, vC);
		}

		// vector<ColoredPoint> vCp = m_fastLivo.getColoredMap();
		// for (int i = 0; i < vCp.size(); i++)
		// {
		// 	ColoredPoint *pCp = &vCp[i];
		// 	vFloat3 vP(pCp->x, pCp->y, pCp->z);
		// 	vFloat3 vC(pCp->r, pCp->g, pCp->b);

		// 	m_pPCout->add(vP, vC);
		// }
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
