/*
 * _OpenPose.h
 *
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Darknet__OpenPose_H_
#define OpenKAI_src_DNN_Darknet__OpenPose_H_

#include "_DetectorBase.h"

#ifdef USE_OPENCV

using namespace cv::dnn;

namespace kai
{

//#define MPI
#define COCO

#ifdef MPI
const int POSE_PAIRS[14][2] =
{
    {0,1}, {1,2}, {2,3},
    {3,4}, {1,5}, {5,6},
    {6,7}, {1,14}, {14,8}, {8,9},
    {9,10}, {14,11}, {11,12}, {12,13}
};

string protoFile = "pose/mpi/pose_deploy_linevec_faster_4_stages.prototxt";
string weightsFile = "pose/mpi/pose_iter_160000.caffemodel";

#define OP_N_POINTS 15
#endif

#ifdef COCO
const int POSE_PAIRS[17][2] =
{
    {1,2}, {1,5}, {2,3},
    {3,4}, {5,6}, {6,7},
    {1,8}, {8,9}, {9,10},
    {1,11}, {11,12}, {12,13},
    {1,0}, {0,14},
    {14,16}, {0,15}, {15,17}
};

#define OP_N_POINTS 18

#endif

class _OpenPose: public _DetectorBase
{
public:
	_OpenPose();
	~_OpenPose();

	bool init(void* pKiss);
	bool start(void);
	void cvDraw(void* pWindow);
	int check(void);

private:
	void detect(void);
	void update(void);
	static void* getUpdate(void* This)
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
	float m_thr;
	Mat m_blob;
	vector<string> m_vLayerName;

	int m_iBackend;
	int m_iTarget;

	Mat m_mDebug;
};

}
#endif
#endif
