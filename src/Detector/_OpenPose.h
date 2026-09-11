/*
 * _OpenPose.h
 *
 *  Created on: Jan 11, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_DNN_Darknet__OpenPose_H_
#define OpenKAI_src_DNN_Darknet__OpenPose_H_

#include "_DetectorBase.h"
using namespace cv::dnn;

namespace kai
{

// #define MPI
#define COCO

#ifdef MPI
	const int POSE_PAIRS[14][2] =
		{
			{0, 1}, {1, 2}, {2, 3}, {3, 4}, {1, 5}, {5, 6}, {6, 7}, {1, 14}, {14, 8}, {8, 9}, {9, 10}, {14, 11}, {11, 12}, {12, 13}};

	string protoFile = "pose/mpi/pose_deploy_linevec_faster_4_stages.prototxt";
	string weightsFile = "pose/mpi/pose_iter_160000.caffemodel";

#define OP_N_POINTS 15
#endif

#ifdef COCO
	const int POSE_PAIRS[17][2] =
		{
			{1, 2}, {1, 5}, {2, 3}, {3, 4}, {5, 6}, {6, 7}, {1, 8}, {8, 9}, {9, 10}, {1, 11}, {11, 12}, {12, 13}, {1, 0}, {0, 14}, {14, 16}, {0, 15}, {15, 17}};

#define OP_N_POINTS 18

#endif

	class _OpenPose : public _DetectorBase
	{
	public:
		_OpenPose();
		~_OpenPose();

		virtual bool init(const json &j);
		virtual bool start(void);
		virtual void draw(void *pFrame);
		virtual bool check(void);

	private:
		void detect(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_OpenPose *)This)->update();
			return NULL;
		}

	protected:
		cv::dnn::Net m_net;
		int m_nW = 368;
		int m_nH = 368;
		bool m_bSwapRB = false;
		float m_scale = 1.0 / 255.0;
		vInt3 m_vMean;
		float m_thr = 0.1;
		Mat m_blob;
		vector<string> m_vLayerName;

		int m_iBackend = dnn::DNN_BACKEND_OPENCV;
		int m_iTarget = dnn::DNN_TARGET_CPU;

		Mat m_mDebug;
	};

}
#endif
