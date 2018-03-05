/*
 * _Lane.h
 *
 *  Created on: Mar 5, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector_Lane_H_
#define OpenKAI_src_Detector_Lane_H_

#include "../Base/common.h"
#include "../Vision/_VisionBase.h"

namespace kai
{

class _Lane: public _ThreadBase
{
public:
	_Lane();
	virtual ~_Lane();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);
	bool draw(void);

	Mat deNoise(Mat imgInput); // Apply Gaussian blurring to the input Image
	Mat edgeDetector(Mat imgNoise); // Filter the image to obtain only edges
	Mat mask(Mat imgEdges); // Mask the edges image to only care about ROI
	vector<Vec4i> houghLines(Mat imggMask); // Detect Hough lines in masked edges image
	vector<vector<Vec4i> > lineSeparation(vector<Vec4i> vLines, Mat imgEdges); // Sprt detected lines by their slope into right and left lines
	vector<Point> regression(vector<vector<Vec4i> > vLRlines, Mat imgInput);  // Get only one line for each side of the lane
	string predictTurn(); // Determine if the lane is turning or not by calculating the position of the vanishing point

private:
	void detectLane(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Lane*) This)->update();
		return NULL;
	}

	_VisionBase*	m_pVision;
	Frame*			m_pFrame;

	double m_imgSize;
	double m_imgCenter;
	bool m_bLeftB; 	// If there's left boundary of lane detected
	bool m_bRightB; // If there's right boundary of lane detected
	Point m_rightB; // Members of both line equations of the lane boundaries:
	Point m_leftB;
	double m_rightM;  // y = m*x + b
	double m_leftM;


	Mat m_frame;
    Mat m_imgDenoise;
    Mat m_imgEdges;
    Mat m_imgMask;
    Mat m_imgLines;
    vector<Vec4i> m_vLines;
    vector<vector<Vec4i> > m_vLRlines;
    vector<Point> m_vLane;
    string m_turn;
};

}
#endif
