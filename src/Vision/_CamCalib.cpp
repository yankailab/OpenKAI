#include "_CamCalib.h"

namespace kai
{

    _CamCalib::_CamCalib()
    {
        m_path = "";
        m_vChessBoardSize.set(9, 6);
        m_squareSize = 1.0;
    }

    _CamCalib::~_CamCalib()
    {
    }

    bool _CamCalib::init(const json &j)
    {
        IF_F(!this->_ModuleBase::init(j));

        jVar(j, "path", m_path);
        jVec<int>(j, "vChessBoardSize", m_vChessBoardSize);
        jVar(j, "squareSize", m_squareSize);

        return true;
    }

    bool _CamCalib::start(void)
    {
        NULL_F(m_pT);
        return m_pT->start(getUpdate, this);
    }

    void _CamCalib::update(void)
    {
        IF_(m_path.empty());

        calibRGB(m_path.c_str());
        exit(0);
    }

    bool _CamCalib::calibRGB(const char *pPath)
    {
        vector<vector<Point3f>> vvPobj; // 3D points for each checkerboard image
        vector<vector<Point2f>> vvPimg; // 2D points for each checkerboard image
        vector<Point3f> vObj;           // world coordinates for 3D points

        for (int i = 0; i < m_vChessBoardSize.x; i++)
        {
            for (int j = 0; j < m_vChessBoardSize.y; j++)
                vObj.push_back(cv::Point3f(j * m_squareSize, i * m_squareSize, 0));
        }

        vector<cv::String> vImgs;
        string path(pPath);
        cv::glob(path, vImgs);

        Mat m, mGray;
        vector<Point2f> vPcorner; // vector to store the pixel coordinates of detected checker board corners
        bool bSuccess;

        for (int i{0}; i < vImgs.size(); i++)
        {
            m = imread(vImgs[i]);
            cv::cvtColor(m, mGray, cv::COLOR_BGR2GRAY);

            // If desired number of corners are found in the image then bSuccess = true
            bSuccess = cv::findChessboardCorners(mGray,
                                                 cv::Size(m_vChessBoardSize.y, m_vChessBoardSize.x),
                                                 vPcorner,
                                                 cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);
            if (bSuccess)
            {
                TermCriteria criteria(TermCriteria::EPS | TermCriteria::MAX_ITER, 30, 0.001);

                // refining pixel coordinates for given 2d points.
                cornerSubPix(mGray, vPcorner, cv::Size(11, 11), cv::Size(-1, -1), criteria);

                // Displaying the detected corner points on the checker board
                drawChessboardCorners(m, cv::Size(m_vChessBoardSize.y, m_vChessBoardSize.x), vPcorner, bSuccess);

                vvPobj.push_back(vObj);
                vvPimg.push_back(vPcorner);
            }

            imshow("CalibrationImages", m);
            waitKey(30);
        }

        IF_F(vvPobj.empty());

        Mat mR, mT;
        calibrateCamera(vvPobj, vvPimg, cv::Size(mGray.rows, mGray.cols), m_mC, m_mD, mR, mT);
        m_mC.at<double>(0, 0) /= (double)mGray.cols; // Fx
        m_mC.at<double>(1, 1) /= (double)mGray.rows; // Fy
        m_mC.at<double>(0, 2) /= (double)mGray.cols; // Cx
        m_mC.at<double>(1, 2) /= (double)mGray.rows; // Cy

        cout << "Camera Matrix : " << m_mC << endl;
        cout << "DistCoeffs : " << m_mD << endl;
        cout << "Rotation vector : " << mR << endl;
        cout << "Translation vector : " << mT << endl;

        return true;
    }

    Mat _CamCalib::mC(void)
    {
        return m_mC;
    }

    Mat _CamCalib::mD(void)
    {
        return m_mD;
    }
}
