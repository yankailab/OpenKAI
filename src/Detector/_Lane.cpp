/*
 * CameraMarkerDetect.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_Lane.h"

namespace kai
{

_Lane::_Lane()
{
	m_pVision = NULL;
	m_pFrame = NULL;

	m_imgSize = 0.0;
	m_imgCenter = 0.0;

	m_bLeftB = false;
	m_bRightB = false;

	m_leftM = 0.0;
	m_rightM = 0.0;

}

_Lane::~_Lane()
{
}

bool _Lane::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	m_pFrame = new Frame();

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

		detectLane();

		this->autoFPSto();
	}
}

void _Lane::detectLane(void)
{
	NULL_(m_pVision);
	m_pFrame->update(m_pVision->bgr());
	IF_(m_pFrame->empty());

	// Denoise the image using a Gaussian filter
	m_imgDenoise = deNoise(*m_pFrame->getCMat());

	// Detect edges in the image
	m_imgEdges = edgeDetector(m_imgDenoise);

	// Mask the image so that we only get the ROI
	m_imgMask = mask(m_imgEdges);

	// Obtain Hough lines in the cropped image
	m_vLines = houghLines(m_imgMask);

	IF_(m_vLines.empty());

	// Separate lines into left and right lines
	m_vLRlines = lineSeparation(m_vLines, m_imgEdges);

	// Apply regression to obtain only one line for each side of the lane
	m_vLane = regression(m_vLRlines, m_frame);

	// Predict the turn by determining the vanishing point of the the lines
	m_turn = predictTurn();
}

bool _Lane::draw(void)
{
	IF_F(!this->_ThreadBase::draw());

	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->getCMat();
	IF_F(pMat->empty());

	NULL_F(m_pFrame);
	Mat* pInput = m_pFrame->getCMat();
	NULL_F(pInput);
	IF_F(pInput->empty());

	IF_F(m_vLines.empty());

	Mat output;
	vector<Point> m_vPolyPoints;

	// Create the transparent polygon for a better visualization of the lane
	pInput->copyTo(output);
	m_vPolyPoints.push_back(m_vLane[2]);
	m_vPolyPoints.push_back(m_vLane[0]);
	m_vPolyPoints.push_back(m_vLane[1]);
	m_vPolyPoints.push_back(m_vLane[3]);
	fillConvexPoly(output, m_vPolyPoints, Scalar(0, 0, 255), CV_AA, 0);
	cv::addWeighted(output, 0.3, *pInput, 1.0 - 0.3, 0, *pMat);

	// Plot both lines of the lane boundary
	line(*pMat, m_vLane[0], m_vLane[1], Scalar(0, 255, 255), 5, CV_AA);
	line(*pMat, m_vLane[2], m_vLane[3], Scalar(0, 255, 255), 5, CV_AA);

	// Plot the turn message
	pWin->addMsg(&m_turn);

	return true;
}

// IMAGE BLURRING
/**
 *@brief Apply gaussian filter to the input image to denoise it
 *@param inputImage is the frame of a video in which the
 *@param lane is going to be detected
 *@return Blurred and denoised image
 */
Mat _Lane::deNoise(Mat imgInput)
{
	Mat output;

	GaussianBlur(imgInput, output, Size(3, 3), 0, 0);

	return output;
}

// EDGE DETECTION
/**
 *@brief Detect all the edges in the blurred frame by filtering the image
 *@param img_noise is the previously blurred frame
 *@return Binary image with only the edges represented in white
 */
Mat _Lane::edgeDetector(Mat imgNoise)
{
	Mat output;
	Mat kernel;
	Point anchor;

	// Convert image from RGB to gray
	cv::cvtColor(imgNoise, output, COLOR_RGB2GRAY);
	// Binarize gray image
	cv::threshold(output, output, 140, 255, THRESH_BINARY);

	// Create the kernel [-1 0 1]
	// This kernel is based on the one found in the
	// Lane Departure Warning System by Mathworks
	anchor = Point(-1, -1);
	kernel = Mat(1, 3, CV_32F);
	kernel.at<float>(0, 0) = -1;
	kernel.at<float>(0, 1) = 0;
	kernel.at<float>(0, 2) = 1;

	// Filter the binary image to obtain the edges
	filter2D(output, output, -1, kernel, anchor, 0, BORDER_DEFAULT);

	return output;
}

// MASK THE EDGE IMAGE
/**
 *@brief Mask the image so that only the edges that form part of the lane are detected
 *@param img_edges is the edges image from the previous function
 *@return Binary image with only the desired edges being represented
 */
Mat _Lane::mask(Mat imgEdges)
{
	Mat output;
	Mat mask = Mat::zeros(imgEdges.size(), imgEdges.type());
	Point pts[4] =
	{ Point(210, 720), Point(550, 450), Point(717, 450), Point(1280, 720) };

	// Create a binary polygon mask
	fillConvexPoly(mask, pts, 4, Scalar(255, 0, 0));
	// Multiply the edges image and the mask to get the output
	cv::bitwise_and(imgEdges, mask, output);

	return output;
}

// HOUGH LINES
/**
 *@brief Obtain all the line segments in the masked images which are going to be part of the lane boundaries
 *@param img_mask is the masked binary image from the previous function
 *@return Vector that contains all the detected lines in the image
 */
vector<Vec4i> _Lane::houghLines(Mat imgMask)
{
	vector<Vec4i> line;

	// rho and theta are selected by trial and error
	HoughLinesP(imgMask, line, 1, CV_PI / 180, 20, 20, 30);

	return line;
}

// SORT RIGHT AND LEFT LINES
/**
 *@brief Sort all the detected Hough lines by slope.
 *@brief The lines are classified into right or left depending
 *@brief on the sign of their slope and their approximate location
 *@param lines is the vector that contains all the detected lines
 *@param img_edges is used for determining the image center
 *@return The output is a vector(2) that contains all the classified lines
 */
vector<vector<Vec4i> > _Lane::lineSeparation(vector<Vec4i> vLines, Mat imgEdges)
{
	vector<vector<Vec4i> > output(2);
	size_t j = 0;
	Point ini;
	Point fini;
	double slope_thresh = 0.3;
	vector<double> slopes;
	vector<Vec4i> selected_lines;
	vector<Vec4i> right_lines, left_lines;

	// Calculate the slope of all the detected lines
	for (auto i : vLines)
	{
		ini = Point(i[0], i[1]);
		fini = Point(i[2], i[3]);

		// Basic algebra: slope = (y1 - y0)/(x1 - x0)
		double slope =
				(static_cast<double>(fini.y) - static_cast<double>(ini.y))
						/ (static_cast<double>(fini.x)
								- static_cast<double>(ini.x) + 0.00001);

		// If the slope is too horizontal, discard the line
		// If not, save them  and their respective slope
		if (abs(slope) > slope_thresh)
		{
			slopes.push_back(slope);
			selected_lines.push_back(i);
		}
	}

	// Split the lines into right and left lines
	m_imgCenter = static_cast<double>((imgEdges.cols / 2));
	while (j < selected_lines.size())
	{
		ini = Point(selected_lines[j][0], selected_lines[j][1]);
		fini = Point(selected_lines[j][2], selected_lines[j][3]);

		// Condition to classify line as left side or right side
		if (slopes[j] > 0 && fini.x > m_imgCenter && ini.x > m_imgCenter)
		{
			right_lines.push_back(selected_lines[j]);
			m_bRightB = true;
		}
		else if (slopes[j] < 0 && fini.x < m_imgCenter && ini.x < m_imgCenter)
		{
			left_lines.push_back(selected_lines[j]);
			m_bLeftB = true;
		}
		j++;
	}

	output[0] = right_lines;
	output[1] = left_lines;

	return output;
}

// REGRESSION FOR LEFT AND RIGHT LINES
/**
 *@brief Regression takes all the classified line segments initial and final points and fits a new lines out of them using the method of least squares.
 *@brief This is done for both sides, left and right.
 *@param left_right_lines is the output of the lineSeparation function
 *@param inputImage is used to select where do the lines will end
 *@return output contains the initial and final points of both lane boundary lines
 */
vector<Point> _Lane::regression(vector<vector<Vec4i> > vLRlines, Mat imgInput)
{
	vector<Point> output(4);
	Point ini;
	Point fini;
	Point ini2;
	Point fini2;
	Vec4d right_line;
	Vec4d left_line;
	vector<Point> right_pts;
	vector<Point> left_pts;

	// If right lines are being detected, fit a line using all the init and final points of the lines
	if (m_bRightB == true)
	{
		for (auto i : vLRlines[0])
		{
			ini = Point(i[0], i[1]);
			fini = Point(i[2], i[3]);

			right_pts.push_back(ini);
			right_pts.push_back(fini);
		}

		if (right_pts.size() > 0)
		{
			// The right line is formed here
			fitLine(right_pts, right_line, CV_DIST_L2, 0, 0.01, 0.01);
			m_rightM = right_line[1] / right_line[0];
			m_rightB = Point(right_line[2], right_line[3]);
		}
	}

	// If left lines are being detected, fit a line using all the init and final points of the lines
	if (m_bLeftB == true)
	{
		for (auto j : vLRlines[1])
		{
			ini2 = Point(j[0], j[1]);
			fini2 = Point(j[2], j[3]);

			left_pts.push_back(ini2);
			left_pts.push_back(fini2);
		}

		if (left_pts.size() > 0)
		{
			// The left line is formed here
			fitLine(left_pts, left_line, CV_DIST_L2, 0, 0.01, 0.01);
			m_leftM = left_line[1] / left_line[0];
			m_leftB = Point(left_line[2], left_line[3]);
		}
	}

	// One the slope and offset points have been obtained, apply the line equation to obtain the line points
	int ini_y = imgInput.rows;
	int fin_y = 470;

	double right_ini_x = ((ini_y - m_rightB.y) / m_rightM) + m_rightB.x;
	double right_fin_x = ((fin_y - m_rightB.y) / m_rightM) + m_rightB.x;

	double left_ini_x = ((ini_y - m_leftB.y) / m_leftM) + m_leftB.x;
	double left_fin_x = ((fin_y - m_leftB.y) / m_leftM) + m_leftB.x;

	output[0] = Point(right_ini_x, ini_y);
	output[1] = Point(right_fin_x, fin_y);
	output[2] = Point(left_ini_x, ini_y);
	output[3] = Point(left_fin_x, fin_y);

	return output;
}

// TURN PREDICTION
/**
 *@brief Predict if the lane is turning left, right or if it is going straight
 *@brief It is done by seeing where the vanishing point is with respect to the center of the image
 *@return String that says if there is left or right turn or if the road is straight
 */
string _Lane::predictTurn()
{
	string output;
	double vanish_x;
	double thr_vp = 10;

	// The vanishing point is the point where both lane boundary lines intersect
	vanish_x = static_cast<double>(((m_rightM * m_rightB.x)
			- (m_leftM * m_leftB.x) - m_rightB.y + m_leftB.y)
			/ (m_rightM - m_leftM));

	// The vanishing points location determines where is the road turning
	if (vanish_x < (m_imgCenter - thr_vp))
		output = "Left Turn";
	else if (vanish_x > (m_imgCenter + thr_vp))
		output = "Right Turn";
	else if (vanish_x >= (m_imgCenter - thr_vp)
			&& vanish_x <= (m_imgCenter + thr_vp))
		output = "Straight";

	return output;
}

}
