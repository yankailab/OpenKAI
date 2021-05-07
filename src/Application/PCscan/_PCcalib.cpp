/*
 * _PCcalib.cpp
 *
 *  Created on: May 6, 2021
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#include "_PCcalib.h"

namespace kai
{

	_PCcalib::_PCcalib()
	{
		m_pPS = NULL;
		m_pUIstate = NULL;
		m_modelName = "PCMODEL";
		m_pTrgb = NULL;
		m_pV = NULL;

		m_bFullScreen = false;
		m_mouseMode = 0;
		m_rDummyDome = 1000.0;

		m_fProcess.clearAll();
	}

	_PCcalib::~_PCcalib()
	{
	}

	bool _PCcalib::init(void *pKiss)
	{
		IF_F(!this->_PCviewer::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("bFullScreen", &m_bFullScreen);
		pK->v("mouseMode", &m_mouseMode);
		pK->v("rDummyDome", &m_rDummyDome);

		string n;

		n = "";
		pK->v("_PCstream", &n);
		m_pPS = (_PCstream *)(pK->getInst(n));

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));

		Kiss *pKr = pK->child("threadRGB");
		IF_F(pKr->empty());
		m_pTrgb = new _Thread();
		if (!m_pTrgb->init(pKr))
		{
			DEL(m_pTrgb);
			return false;
		}

		return true;
	}

	bool _PCcalib::start(void)
	{
		NULL_F(m_pT);
		IF_F(!m_pT->start(getUpdate, this));

		NULL_F(m_pTui);
		IF_F(!m_pTui->start(getUpdateUI, this));

		NULL_F(m_pTrgb);
		IF_F(!m_pTrgb->start(getUpdateRGB, this));

		return true;
	}

	int _PCcalib::check(void)
	{
		NULL__(m_pPS, -1);
		NULL__(m_pV, -1);

		return this->_PCviewer::check();
	}

	void _PCcalib::update(void)
	{
		m_pT->sleepT(0);

		startScan();

		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			updateScan();
			updateProcess();

			m_pT->autoFPSto();
		}
	}

	void _PCcalib::startScan(void)
	{
		IF_(check() < 0);

		m_pPS->clear();

		removeUIpc();
		addDummyDome(m_sPC.next(), m_pPS->nP(), m_rDummyDome);
		updatePC();
		addUIpc(*m_sPC.get());
	}

	void _PCcalib::updateScan(void)
	{
		IF_(check() < 0);

		readAllPC();
		PointCloud *pPC = m_sPC.get();
		pPC->normals_.clear();
		int n = pPC->points_.size();
		IF_(n <= 0);
		int nP = m_pPS->nP();

		m_aabb = pPC->GetAxisAlignedBoundingBox();

		PointCloud pc = *pPC;
		if (n < nP)
		{
			addDummyDome(&pc, nP - n, m_rDummyDome);
		}
		else if (n > nP)
		{
			int d = n - nP;
			pc.points_.erase(pc.points_.end() - d, pc.points_.end());
			pc.colors_.erase(pc.colors_.end() - d, pc.colors_.end());
		}

		updateUIpc(pc);
	}

	void _PCcalib::updateProcess(void)
	{
		IF_(check() < 0);

		if (m_fProcess.b(pcfCalibReset))
		{
			startScan();
		}
	}

	void _PCcalib::addUIpc(const PointCloud &pc)
	{
		IF_(pc.IsEmpty());

		m_spWin->AddPointCloud(m_modelName,
							   make_shared<t::geometry::PointCloud>(
								   t::geometry::PointCloud::FromLegacyPointCloud(
									   pc,
									   core::Dtype::Float32)));
	}

	void _PCcalib::updateUIpc(const PointCloud &pc)
	{
		IF_(pc.IsEmpty());

		m_spWin->UpdatePointCloud(m_modelName,
								  make_shared<t::geometry::PointCloud>(
									  t::geometry::PointCloud::FromLegacyPointCloud(
										  pc,
										  core::Dtype::Float32)));
	}

	void _PCcalib::removeUIpc(void)
	{
		m_spWin->RemoveGeometry(m_modelName);
	}

	void _PCcalib::updateRGB(void)
	{
		while (m_pTrgb->bRun())
		{
			m_pTrgb->autoFPSfrom();

			updateRGBtransform();

			m_pTrgb->autoFPSto();
		}
	}

	void _PCcalib::updateRGBtransform(void)
	{
	}

	void _PCcalib::updateUI(void)
	{
		auto &app = gui::Application::GetInstance();
		app.Initialize(m_pathRes.c_str());

		m_spWin = std::make_shared<visualizer::PCcalibUI>(*this->getName(), 2000, 1000);
		app.AddWindow(m_spWin);

		m_spWin->SetCbLoadImgs(OnLoadImgs, (void *)this);
		m_spWin->SetCbResetPC(OnResetPC, (void *)this);

		m_pUIstate = m_spWin->getUIState();
		m_pUIstate->m_mouseMode = (visualization::gui::SceneWidget::Controls)m_mouseMode;
		m_spWin->UpdateUIstate();
		m_spWin->SetFullScreen(m_bFullScreen);
		m_aabb = createDefaultAABB();
		camBound(m_aabb);
		updateCamProj();
		updateCamPose();

		m_pT->wakeUp();
		app.Run();
		exit(0);
	}

	void _PCcalib::updateCamProj(void)
	{
		IF_(check() < 0);
		IF_(!m_spWin);

		m_spWin->CamSetProj(m_camProj.m_fov,
							m_camProj.m_vNF.x,
							m_camProj.m_vNF.y,
							m_camProj.m_fovType);
	}

	void _PCcalib::updateCamPose(void)
	{
		IF_(check() < 0);
		IF_(!m_spWin);

		m_spWin->CamSetPose(m_cam.m_vLookAt.v3f(),
							m_cam.m_vEye.v3f(),
							m_cam.m_vUp.v3f());
	}

	void _PCcalib::camBound(const AxisAlignedBoundingBox &aabb)
	{
		IF_(check() < 0);
		IF_(!m_spWin);

		m_spWin->CamAutoBound(aabb, m_vCoR.v3f());
	}

	AxisAlignedBoundingBox _PCcalib::createDefaultAABB(void)
	{
		PointCloud pc;
		pc.points_.push_back(Vector3d(0, 0, 1));
		pc.points_.push_back(Vector3d(0, 0, -1));
		pc.points_.push_back(Vector3d(0, 1, 0));
		pc.points_.push_back(Vector3d(0, -1, 0));
		pc.points_.push_back(Vector3d(1, 0, 0));
		pc.points_.push_back(Vector3d(-1, 0, 0));
		return pc.GetAxisAlignedBoundingBox();
	}

	void _PCcalib::OnLoadImgs(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCcalib *pV = (_PCcalib *)pPCV;

		pV->calibRGB((const char *)pD);
	}

	void _PCcalib::OnResetPC(void *pPCV, void *pD)
	{
		NULL_(pPCV);
		NULL_(pD);
		_PCcalib *pV = (_PCcalib *)pPCV;

		pV->m_fProcess.set(pcfCalibReset);
	}

	void _PCcalib::calibRGB(const char *pPath)
	{
		vector<vector<Point3f>> objpoints;		// Creating vector to store vectors of 3D points for each checkerboard image
		vector<vector<Point2f>> imgpoints;		// Creating vector to store vectors of 2D points for each checkerboard image
		vector<Point3f> objp;		// Defining the world coordinates for 3D points

		for (int i{0}; i < CHECKERBOARD[1]; i++)
		{
			for (int j{0}; j < CHECKERBOARD[0]; j++)
				objp.push_back(cv::Point3f(j, i, 0));
		}

		// Extracting path of individual image stored in a given directory
		vector<cv::String> images;
		string path(pPath);
		cv::glob(path, images);

		Mat frame, gray;
		vector<Point2f> corner_pts;	// vector to store the pixel coordinates of detected checker board corners
		bool success;

		for (int i{0}; i < images.size(); i++)
		{
			frame = cv::imread(images[i]);
			cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

			// If desired number of corners are found in the image then success = true
			success = cv::findChessboardCorners(gray, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);

			/*
		     * If desired number of corner are detected,
     		 * we refine the pixel coordinates and display 
    		 * them on the images of checker board
		    */
			if (success)
			{
				cv::TermCriteria criteria(cv::TermCriteria::EPS | cv::TermCriteria::MAX_ITER, 30, 0.001);

				// refining pixel coordinates for given 2d points.
				cv::cornerSubPix(gray, corner_pts, cv::Size(11, 11), cv::Size(-1, -1), criteria);

				// Displaying the detected corner points on the checker board
				cv::drawChessboardCorners(frame, cv::Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, success);

				objpoints.push_back(objp);
				imgpoints.push_back(corner_pts);
			}

			cv::imshow("CalibrationImages", frame);
			cv::waitKey(0);
		}

		Mat cameraMatrix, distCoeffs, R, T;
		cv::calibrateCamera(objpoints, imgpoints, cv::Size(gray.rows, gray.cols), cameraMatrix, distCoeffs, R, T);

		cout << "cameraMatrix : " << cameraMatrix << std::endl;
		cout << "distCoeffs : " << distCoeffs << std::endl;
		cout << "Rotation vector : " << R << std::endl;
		cout << "Translation vector : " << T << std::endl;

		// Trying to undistort the image using the camera parameters obtained from calibration

		cv::Mat image;
		image = cv::imread(images[0]);
		cv::Mat dst, map1, map2, new_camera_matrix;
		cv::Size imageSize(cv::Size(image.cols, image.rows));

		// Refining the camera matrix using parameters obtained by calibration
		new_camera_matrix = cv::getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0);

		// Method 1 to undistort the image
//		cv::undistort(frame, dst, new_camera_matrix, distCoeffs, new_camera_matrix);

		// Method 2 to undistort the image
		cv::initUndistortRectifyMap(cameraMatrix, distCoeffs, cv::Mat(), new_camera_matrix, imageSize, CV_16SC2, map1, map2);

		cv::remap(frame, dst, map1, map2, cv::INTER_LINEAR);

	}

}
#endif
