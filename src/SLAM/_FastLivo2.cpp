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
	}

	_FastLivo2::~_FastLivo2()
	{
	}

	bool _FastLivo2::init(const json &j)
	{
		IF_F(!this->_SLAMbase::init(j));

		//		jKv<int>(j, "vSize", m_vSize);
		if (!core.init(""))
		{
			std::cerr << "init failed\n";
			return false;
		}

		core.setImageEnabled(true);

		W = 640;
		H = 480;
		const double fx = 520.0, fy = 520.0, cx = 320.0, cy = 240.0;
		std::array<double, 5> dist = {0.0, 0.0, 0.0, 0.0, 0.0}; // k1,k2,p1,p2,k3 (fill real values if you have them)
		core.setCameraPinholeDistorted(W, H, fx, fy, cx, cy, dist);

		// 3) extrinsics
		// IMU<-LiDAR: p_imu = R_il * p_lidar + t_il
		std::array<double, 9> R_il = {1, 0, 0, 0, 1, 0, 0, 0, 1};
		std::array<double, 3> t_il = {0, 0, 0};
		core.setImuToLidarExtrinsic(R_il, t_il);

		// Camera<-LiDAR: p_cam = R_cl * p_lidar + t_cl
		std::array<double, 9> R_cl = {1, 0, 0, 0, 1, 0, 0, 0, 1};
		std::array<double, 3> t_cl = {0.05, 0.0, 0.0}; // example 5cm
		core.setLidarToCameraExtrinsic(R_cl, t_cl);

		// 4) colored map maintenance options
		// downsample_every_n_frames, max_map_points, voxel_leaf_size_m
		core.configureColoredMapMaintenance(20, 300000, 0.10);

		// 5) feed loop (synthetic demo)
		t = 0.0;
		T_end = 5.0;
		imu_dt = 0.005; // 200 Hz
		scan_dt = 0.1;	// 10 Hz
		img_dt = 0.1;	// 10 Hz
		next_scan = 0.0;
		next_img = 0.0;

		return true;
	}

	bool _FastLivo2::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_SLAMbase::link(j, pM));

		// string n = "";
		// jKv(j, "_VisionBase", n);
		// m_pV = (_VisionBase *)(pM->findModule(n));
		// NULL_F(m_pV);

		return true;
	}

	bool _FastLivo2::check(void)
	{
		NULL_F(m_pV);
		NULL_F(m_pV->getFrameRGB());

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

			updateFastLivo2();
		}
	}

	void _FastLivo2::updateFastLivo2(void)
	{
		//		IF_(!check());

		// static const double usecBase = 1.0 / ((double)SEC_2_USEC);

		// Mat mGray;
		// m_pV->getFrameRGB()->m()->copyTo(mGray);
		// IF_(mGray.empty());

		// IMU sample
		ImuSample imu;
		imu.t = t;
		imu.acc = Eigen::Vector3d(0, 0, 9.81);	// gravity-ish
		imu.gyro = Eigen::Vector3d(0, 0, 0.02); // slow yaw
		core.pushImu(imu);

		// LiDAR scan
		if (t >= next_scan)
		{
			auto scan = makeTestScan(t, t + scan_dt, 20000);
			core.pushLidar(scan);
			next_scan += scan_dt;
		}

		// BGR image
		if (t >= next_img)
		{
			auto img = makeTestBGR(t, W, H);
			core.pushImage(img);
			next_img += img_dt;
		}

		// run one step (may return false if not enough data yet)
		(void)core.spinOnce();

		t += imu_dt;
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
		string msg;
		if (m_bTracking)
		{
			msg = "Tracking";
		}
		else
		{
			msg = "Tracking lost";
		}
		pC->addMsg(msg);

		msg = "Global pos: x=" + f2str(m_vT.x) + ", y=" + f2str(m_vT.y) + ", z=" + f2str(m_vT.z);
		pC->addMsg(msg);
	}

}
