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

	if(m_vSize.x != pInput->cols || m_vSize.y != pInput->rows)
	{
		m_vSize.x = pInput->cols;
		m_vSize.y = pInput->rows;
		updateVisionSize();
	}

	cv::warpPerspective(*pInput, m_mOverhead, m_mPerspective, m_mOverhead.size(), cv::INTER_LINEAR);

	cv::cvtColor(m_mOverhead, m_mLAB, CV_BGR2Lab);
	cv::cvtColor(m_mOverhead, m_mHSV, CV_BGR2HSV);
	cv::cvtColor(m_mOverhead, m_mHLS, CV_BGR2HLS);

/*
# Settings to run thresholding operations on
        settings = [{'name': 'lab_b', 'cspace': 'LAB', 'channel': 2, 'clipLimit': 2.0, 'threshold': 150},
                    {'name': 'value', 'cspace': 'HSV', 'channel': 2, 'clipLimit': 6.0, 'threshold': 220},
                    {'name': 'lightness', 'cspace': 'HLS', 'channel': 1, 'clipLimit': 2.0, 'threshold': 210}]

        # Perform binary thresholding according to each setting and combine them into one image.
        scores = np.zeros(img.shape[0:2]).astype('uint8')
        for params in settings:
            # Change color space
            color_t = getattr(cv2, 'COLOR_RGB2{}'.format(params['cspace']))
            gray = cv2.cvtColor(img, color_t)[:, :, params['channel']]

            # Normalize regions of the image using CLAHE
            clahe = cv2.createCLAHE(params['clipLimit'], tileGridSize=(8, 8))
            norm_img = clahe.apply(gray)

            # Threshold to binary
            ret, binary = cv2.threshold(norm_img, params['threshold'], 1, cv2.THRESH_BINARY)

            scores += binary

            # Save images
            self.viz_save(params['name'], gray)
            self.viz_save(params['name'] + '_binary', binary)

        return cv2.normalize(scores, None, 0, 255, cv2.NORM_MINMAX)

 */


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

//// EDGE DETECTION
///**
// *@brief Detect all the edges in the blurred frame by filtering the image
// *@param img_noise is the previously blurred frame
// *@return Binary image with only the edges represented in white
// */
//void _Lane::edgeDetector(void)
//{
//	Mat output;
//	Mat kernel;
//	Point anchor;
//
//	// Convert image from RGB to gray
//	cv::cvtColor(m_imgDenoise, output, COLOR_RGB2GRAY);
//	// Binarize gray image
//	cv::threshold(output, output, 140, 255, THRESH_BINARY);
//
//	// Create the kernel [-1 0 1]
//	// This kernel is based on the one found in the
//	// Lane Departure Warning System by Mathworks
//	anchor = Point(-1, -1);
//	kernel = Mat(1, 3, CV_32F);
//	kernel.at<float>(0, 0) = -1;
//	kernel.at<float>(0, 1) = 0;
//	kernel.at<float>(0, 2) = 1;
//
//	// Filter the binary image to obtain the edges
//	filter2D(output, output, -1, kernel, anchor, 0, BORDER_DEFAULT);
//
//	m_imgEdges = output;
//}
//
//// MASK THE EDGE IMAGE
///**
// *@brief Mask the image so that only the edges that form part of the lane are detected
// *@param img_edges is the edges image from the previous function
// *@return Binary image with only the desired edges being represented
// */
//void _Lane::mask(void)
//{
//	Mat output;
//	Mat mask = Mat::zeros(m_imgEdges.size(), m_imgEdges.type());
//	Point pts[4] =
//	{ Point(210, 720), Point(550, 450), Point(717, 450), Point(1280, 720) };
//
//	// Create a binary polygon mask
//	fillConvexPoly(mask, pts, 4, Scalar(255, 0, 0));
//	// Multiply the edges image and the mask to get the output
//	cv::bitwise_and(m_imgEdges, mask, output);
//
//	m_imgMask = output;
//}
//
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
