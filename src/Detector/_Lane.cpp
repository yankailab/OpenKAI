/*
 * _Lane.cpp
 *
 *  Created on: Mar 5, 2018
 *      Author: yankai
 */

#include "_Lane.h"

namespace kai
{

_Lane::_Lane()
{
	m_pVision = NULL;

	m_w = 0.1;
	m_h = 0.1;
	m_dW = 0.05;
	m_dH = 0.05;
	m_nBlockX= ((1.0 - m_w) / m_dW) + 1;
	m_nBlockY = ((1.0 - m_h) / m_dH) + 1;
	m_nBlock = m_nBlockX * m_nBlockY;
	m_pBlock = NULL;

	m_roiLT.x = 0.2;
	m_roiLT.y = 0.5;
	m_roiLB.x = 0.0;
	m_roiLB.y = 1.0;
	m_roiRT.x = 0.8;
	m_roiRT.y = 0.5;
	m_roiRB.x = 1.0;
	m_roiRB.y = 1.0;

	m_sizeOverhead.x = 400;
	m_sizeOverhead.y = 200;
	m_vSize.init();

	m_tileClahe = 8;
	m_clipLimLAB = 2.0;
	m_clipLimHSV = 6.0;
	m_clipLimHLS = 2.0;
	m_thrLAB = 150;
	m_thrHSV = 220;
	m_thrHLS = 210;

}

_Lane::~_Lane()
{
}

bool _Lane::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK, w);
	KISSm(pK, h);
	KISSm(pK, dW);
	KISSm(pK, dH);
	F_INFO(pK->v("roiLTx", &m_roiLT.x));
	F_INFO(pK->v("roiLTy", &m_roiLT.y));
	F_INFO(pK->v("roiLBx", &m_roiLB.x));
	F_INFO(pK->v("roiLBy", &m_roiLB.y));
	F_INFO(pK->v("roiRTx", &m_roiRT.x));
	F_INFO(pK->v("roiRTy", &m_roiRT.y));
	F_INFO(pK->v("roiRBx", &m_roiRB.x));
	F_INFO(pK->v("roiRBy", &m_roiRB.y));
	F_INFO(pK->v("overheadW", &m_sizeOverhead.x));
	F_INFO(pK->v("overheadH", &m_sizeOverhead.y));

	m_mOverhead = Mat(Size(m_sizeOverhead.x, m_sizeOverhead.y),CV_8UC3);

	KISSm(pK, tileClahe);
	KISSdm(pK, clipLimLAB);
	KISSdm(pK, clipLimHSV);
	KISSdm(pK, clipLimHLS);
	KISSm(pK, thrLAB);
	KISSm(pK, thrHSV);
	KISSm(pK, thrHLS);

	m_pClaheLAB = cv::createCLAHE(m_clipLimLAB,Size(m_tileClahe,m_tileClahe));
	m_pClaheHSV = cv::createCLAHE(m_clipLimHSV,Size(m_tileClahe,m_tileClahe));
	m_pClaheHLS = cv::createCLAHE(m_clipLimHLS,Size(m_tileClahe,m_tileClahe));

	//create detection area instances
	m_nBlockX= ((1.0 - m_w) / m_dW) + 1;
	m_nBlockY = ((1.0 - m_h) / m_dH) + 1;
	if (m_nBlockX <= 0 || m_nBlockY <= 0)
	{
		LOG_E("nW <=0 || nH <= 0");
		return false;
	}

	m_nBlock = m_nBlockX * m_nBlockY;
	m_pBlock = new LANE_BLOCK[m_nBlock];

	int i,j;
	for (i = 0; i < m_nBlockY; i++)
	{
		for (j = 0; j < m_nBlockX; j++)
		{
			LANE_BLOCK* pB = &m_pBlock[i*m_nBlockY+j];
			pB->init();
			pB->m_fROI.x = j * m_dW;
			pB->m_fROI.z = pB->m_fROI.x + m_w;
			pB->m_fROI.y = i * m_dH;
			pB->m_fROI.w = pB->m_fROI.y + m_h;
		}
	}

	return true;
}

bool _Lane::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_ERROR_F(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInstByName(&iName));
	NULL_Fl(m_pVision, "_VisionBase is NULL");

	return true;
}

bool _Lane::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _Lane::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		detect();

		this->autoFPSto();
	}
}

void _Lane::detect(void)
{
	NULL_(m_pVision);
	Mat* pInput = m_pVision->bgr()->getCMat();
	IF_(pInput->empty());

	//Warp transform to get overhead view
	if(m_vSize.x != pInput->cols || m_vSize.y != pInput->rows)
	{
		m_vSize.x = pInput->cols;
		m_vSize.y = pInput->rows;
		updateVisionSize();
	}

	cv::warpPerspective(*pInput, m_mOverhead, m_mPerspective, m_mOverhead.size(), cv::INTER_LINEAR);

	//Filter image to get binary view
	filter();

	//Update block occupancy
	updateBlock();

	updateLaneBlock();
}

void _Lane::updateLaneBlock(void)
{
	int i,j;



}

void _Lane::updateBlock(void)
{
	int i,j;
	for (i = 0; i < m_nBlockY; i++)
	{
		for (j = 0; j < m_nBlockX; j++)
		{
			LANE_BLOCK* pB = &m_pBlock[i*m_nBlockY+j];

			pB->m_v = cv::countNonZero(m_mBin(pB->m_iROI));
		}
	}
}

void _Lane::filter(void)
{
	vector<Mat> vMat(3);
	Mat tMat;
	Mat tMatBin;
	Mat mAll;

	cv::cvtColor(m_mOverhead, m_mLAB, CV_BGR2Lab);
	split(m_mLAB, vMat);
	m_pClaheLAB->apply(vMat[2],tMat);
	cv::threshold(tMat,tMatBin,m_thrLAB,1,cv::THRESH_BINARY);
	mAll = tMatBin;

	cv::cvtColor(m_mOverhead, m_mHSV, CV_BGR2HSV);
	split(m_mHSV, vMat);
	m_pClaheHSV->apply(vMat[2],tMat);
	cv::threshold(tMat,tMatBin,m_thrHSV,1,cv::THRESH_BINARY);
	mAll += tMatBin;

	cv::cvtColor(m_mOverhead, m_mHLS, CV_BGR2HLS);
	split(m_mHLS, vMat);
	m_pClaheHLS->apply(vMat[1],tMat);
	cv::threshold(tMat,tMatBin,m_thrHLS,1.0,cv::THRESH_BINARY);
	mAll += tMatBin;

	cv::normalize(mAll, m_mBin, 0, 255, cv::NORM_MINMAX);
}

void _Lane::updateVisionSize(void)
{
	Point2f LT = Point2f((float)(m_roiLT.x * m_vSize.x), (float)(m_roiLT.y * m_vSize.y));
	Point2f LB = Point2f((float)(m_roiLB.x * m_vSize.x), (float)(m_roiLB.y * m_vSize.y));
	Point2f RT = Point2f((float)(m_roiRT.x * m_vSize.x), (float)(m_roiRT.y * m_vSize.y));
	Point2f RB = Point2f((float)(m_roiRB.x * m_vSize.x), (float)(m_roiRB.y * m_vSize.y));

	//LT, LB, RB, RT
	Point2f ptsFrom[] = {LT,LB,RB,RT};
	Point2f ptsTo[] = {cv::Point2f(0,0),
					   cv::Point2f(0,(float)m_sizeOverhead.y),
					   cv::Point2f((float)m_sizeOverhead.x, (float)m_sizeOverhead.y),
					   cv::Point2f((float)m_sizeOverhead.x,0)};

	m_mPerspective = getPerspectiveTransform(ptsFrom, ptsTo);

	int i,j;
	for (i = 0; i < m_nBlockY; i++)
	{
		for (j = 0; j < m_nBlockX; j++)
		{
			m_pBlock[i*m_nBlockY+j].f2iROI(m_vSize);
		}
	}

}

bool _Lane::draw(void)
{
	IF_F(!this->_ThreadBase::draw());

	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->getCMat();
	IF_F(pMat->empty());

	if(m_mOverhead.empty())return true;
	imshow("Overhead",m_mOverhead);

	if(m_mBin.empty())return true;
	imshow("Filter",m_mBin);

//	pWin->addMsg();
//	Mat output;
//	vector<Point> m_vPolyPoints;
//
//	// Create the transparent polygon for a better visualization of the lane
//	pMat->copyTo(output);
//	m_vPolyPoints.push_back(m_vLane[2]);
//	m_vPolyPoints.push_back(m_vLane[0]);
//	m_vPolyPoints.push_back(m_vLane[1]);
//	m_vPolyPoints.push_back(m_vLane[3]);
//	fillConvexPoly(output, m_vPolyPoints, Scalar(0, 255, 0), CV_AA, 0);
//	cv::addWeighted(output, 0.1, *pMat, 1.0, 0, *pMat);
//
//	// Plot both lines of the lane boundary
//	line(*pMat, m_vLane[0], m_vLane[1], Scalar(0, 255, 255), 2, CV_AA);
//	line(*pMat, m_vLane[2], m_vLane[3], Scalar(0, 255, 255), 2, CV_AA);

	return true;
}

//// SORT RIGHT AND LEFT LINES
///**
// *@brief Sort all the detected Hough lines by slope.
// *@brief The lines are classified into right or left depending
// *@brief on the sign of their slope and their approximate location
// *@param lines is the vector that contains all the detected lines
// *@param img_edges is used for determining the image center
// *@return The output is a vector(2) that contains all the classified lines
// */
//void _Lane::lineSeparation(void)
//{
//	vector<vector<Vec4i> > output(2);
//	size_t j = 0;
//	Point ini;
//	Point fini;
//	double slope_thresh = 0.3;
//	vector<double> slopes;
//	vector<Vec4i> selected_lines;
//	vector<Vec4i> right_lines, left_lines;
//
//	// Calculate the slope of all the detected lines
//	for (auto i : m_vLines)
//	{
//		ini = Point(i[0], i[1]);
//		fini = Point(i[2], i[3]);
//
//		// Basic algebra: slope = (y1 - y0)/(x1 - x0)
//		double slope =
//				(static_cast<double>(fini.y) - static_cast<double>(ini.y))
//						/ (static_cast<double>(fini.x)
//								- static_cast<double>(ini.x) + 0.00001);
//
//		// If the slope is too horizontal, discard the line
//		// If not, save them  and their respective slope
//		if (abs(slope) > slope_thresh)
//		{
//			slopes.push_back(slope);
//			selected_lines.push_back(i);
//		}
//	}
//
//	// Split the lines into right and left lines
//	m_imgCenter = static_cast<double>((m_imgEdges.cols / 2));
//	while (j < selected_lines.size())
//	{
//		ini = Point(selected_lines[j][0], selected_lines[j][1]);
//		fini = Point(selected_lines[j][2], selected_lines[j][3]);
//
//		// Condition to classify line as left side or right side
//		if (slopes[j] > 0 && fini.x > m_imgCenter && ini.x > m_imgCenter)
//		{
//			right_lines.push_back(selected_lines[j]);
//			m_bRightB = true;
//		}
//		else if (slopes[j] < 0 && fini.x < m_imgCenter && ini.x < m_imgCenter)
//		{
//			left_lines.push_back(selected_lines[j]);
//			m_bLeftB = true;
//		}
//		j++;
//	}
//
//	output[0] = right_lines;
//	output[1] = left_lines;
//
//	m_vLRlines = output;
//}
//
//// REGRESSION FOR LEFT AND RIGHT LINES
///**
// *@brief Regression takes all the classified line segments initial and final points and fits a new lines out of them using the method of least squares.
// *@brief This is done for both sides, left and right.
// *@param left_right_lines is the output of the lineSeparation function
// *@param inputImage is used to select where do the lines will end
// *@return output contains the initial and final points of both lane boundary lines
// */
//void _Lane::regression(void)
//{
//	vector<Point> output(4);
//	Point ini;
//	Point fini;
//	Point ini2;
//	Point fini2;
//	Vec4d right_line;
//	Vec4d left_line;
//	vector<Point> right_pts;
//	vector<Point> left_pts;
//
//	// If right lines are being detected, fit a line using all the init and final points of the lines
//	if (m_bRightB == true)
//	{
//		for (auto i : m_vLRlines[0])
//		{
//			ini = Point(i[0], i[1]);
//			fini = Point(i[2], i[3]);
//
//			right_pts.push_back(ini);
//			right_pts.push_back(fini);
//		}
//
//		if (right_pts.size() > 0)
//		{
//			// The right line is formed here
//			fitLine(right_pts, right_line, CV_DIST_L2, 0, 0.01, 0.01);
//			m_rightM = right_line[1] / right_line[0];
//			m_rightB = Point(right_line[2], right_line[3]);
//		}
//	}
//
//	// If left lines are being detected, fit a line using all the init and final points of the lines
//	if (m_bLeftB == true)
//	{
//		for (auto j : m_vLRlines[1])
//		{
//			ini2 = Point(j[0], j[1]);
//			fini2 = Point(j[2], j[3]);
//
//			left_pts.push_back(ini2);
//			left_pts.push_back(fini2);
//		}
//
//		if (left_pts.size() > 0)
//		{
//			// The left line is formed here
//			fitLine(left_pts, left_line, CV_DIST_L2, 0, 0.01, 0.01);
//			m_leftM = left_line[1] / left_line[0];
//			m_leftB = Point(left_line[2], left_line[3]);
//		}
//	}
//
//	// One the slope and offset points have been obtained, apply the line equation to obtain the line points
//	int ini_y = m_pFrame->getCMat()->rows;
//	int fin_y = 470;
//
//	double right_ini_x = ((ini_y - m_rightB.y) / m_rightM) + m_rightB.x;
//	double right_fin_x = ((fin_y - m_rightB.y) / m_rightM) + m_rightB.x;
//
//	double left_ini_x = ((ini_y - m_leftB.y) / m_leftM) + m_leftB.x;
//	double left_fin_x = ((fin_y - m_leftB.y) / m_leftM) + m_leftB.x;
//
//	output[0] = Point(right_ini_x, ini_y);
//	output[1] = Point(right_fin_x, fin_y);
//	output[2] = Point(left_ini_x, ini_y);
//	output[3] = Point(left_fin_x, fin_y);
//
//	m_vLane = output;
//}


}
