/*
 * _OpenPose.h
 *
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Darknet__OpenPose_H_
#define OpenKAI_src_DNN_Darknet__OpenPose_H_

#include "../Base/common.h"
#include "../Vision/_VisionBase.h"
#include "_DetectorBase.h"

using namespace cv::dnn;

namespace kai
{

struct KeyPoint
{
	KeyPoint(cv::Point point, float probability)
	{
		this->id = -1;
		this->point = point;
		this->probability = probability;
	}

	int id;
	cv::Point point;
	float probability;
};

inline std::ostream& operator <<(std::ostream& os, const KeyPoint& kp)
{
	os << "Id:" << kp.id << ", Point:" << kp.point << ", Prob:"
			<< kp.probability << std::endl;
	return os;
}

struct ValidPair
{
	ValidPair(int aId, int bId, float score)
	{
		this->aId = aId;
		this->bId = bId;
		this->score = score;
	}

	int aId;
	int bId;
	float score;
};

inline std::ostream& operator <<(std::ostream& os, const ValidPair& vp)
{
	os << "A:" << vp.aId << ", B:" << vp.bId << ", score:" << vp.score
			<< std::endl;
	return os;
}

template<class T> std::ostream& operator <<(std::ostream& os,
		const std::vector<T>& v)
{
	os << "[";
	bool first = true;
	for (typename std::vector<T>::const_iterator ii = v.begin(); ii != v.end();
			++ii, first = false)
	{
		if (!first)
			os << ",";
		os << " " << *ii;
	}
	os << "]";
	return os;
}

template<class T> std::ostream& operator <<(std::ostream& os,
		const std::set<T>& v)
{
	os << "[";
	bool first = true;
	for (typename std::set<T>::const_iterator ii = v.begin(); ii != v.end();
			++ii, first = false)
	{
		if (!first)
			os << ",";
		os << " " << *ii;
	}
	os << "]";
	return os;
}

#define OP_N_POINTS 18

const std::string keypointsMapping[] =
{ "Nose", "Neck", "R-Sho", "R-Elb", "R-Wr", "L-Sho", "L-Elb", "L-Wr", "R-Hip",
		"R-Knee", "R-Ank", "L-Hip", "L-Knee", "L-Ank", "R-Eye", "L-Eye",
		"R-Ear", "L-Ear" };

const std::vector<std::pair<int, int>> mapIdx =
{
{ 31, 32 },
{ 39, 40 },
{ 33, 34 },
{ 35, 36 },
{ 41, 42 },
{ 43, 44 },
{ 19, 20 },
{ 21, 22 },
{ 23, 24 },
{ 25, 26 },
{ 27, 28 },
{ 29, 30 },
{ 47, 48 },
{ 49, 50 },
{ 53, 54 },
{ 51, 52 },
{ 55, 56 },
{ 37, 38 },
{ 45, 46 } };

const std::vector<std::pair<int, int>> posePairs =
{
{ 1, 2 },
{ 1, 5 },
{ 2, 3 },
{ 3, 4 },
{ 5, 6 },
{ 6, 7 },
{ 1, 8 },
{ 8, 9 },
{ 9, 10 },
{ 1, 11 },
{ 11, 12 },
{ 12, 13 },
{ 1, 0 },
{ 0, 14 },
{ 14, 16 },
{ 0, 15 },
{ 15, 17 },
{ 2, 17 },
{ 5, 16 } };

class _OpenPose: public _DetectorBase
{
public:
	_OpenPose();
	~_OpenPose();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	int check(void);

private:
	void getKeyPoints(Mat& probMap, double threshold,
			vector<KeyPoint>& keyPoints);
	void populateColorPalette(vector<Scalar>& colors, int nColors);
	void splitNetOutputBlobToParts(Mat& netOutputBlob, const Size& targetSize,
			vector<Mat>& netOutputParts);
	void populateInterpPoints(const Point& a, const Point& b, int numPoints,
			vector<Point>& interpCoords);
	void getValidPairs(const vector<Mat>& netOutputParts,
			const vector<vector<KeyPoint>>& detectedKeypoints,
			vector<vector<ValidPair>>& validPairs, set<int>& invalidPairs);
	void getPersonwiseKeypoints(const vector<vector<ValidPair>>& validPairs,
			const set<int>& invalidPairs,
			vector<vector<int>>& personwiseKeypoints);

	bool detect(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_OpenPose*) This)->update();
		return NULL;
	}

public:
	cv::dnn::Net m_net;
	int m_nW;
	int m_nH;
	bool m_bSwapRB;
	float m_scale;
	vInt3 m_vMean;
	Mat m_blob;
	vector<string> m_vLayerName;

	int m_iBackend;
	int m_iTarget;

	std::vector<cv::Mat> m_netOutputParts;
	std::vector<std::vector<KeyPoint>> m_detectedKeypoints;
	std::vector<KeyPoint> m_keyPointsList;



};

}
#endif
