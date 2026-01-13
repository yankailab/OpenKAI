/*
 * _HandKey.h
 *
 *  Created on: Mar 11, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__HandKey_H_
#define OpenKAI_src_Detector__HandKey_H_

#include "_DetectorBase.h"

using namespace cv::dnn;

namespace kai
{

	const int HANDKEY_POSE_PAIRS[20][2] =
		{
			{0, 1}, {1, 2}, {2, 3}, {3, 4}, // thumb
			{0, 5},
			{5, 6},
			{6, 7},
			{7, 8}, // index
			{0, 9},
			{9, 10},
			{10, 11},
			{11, 12}, // middle
			{0, 13},
			{13, 14},
			{14, 15},
			{15, 16}, // ring
			{0, 17},
			{17, 18},
			{18, 19},
			{19, 20} // small
	};

#define HANDKEY_N_P 22

	class _HandKey : public _DetectorBase
	{
	public:
		_HandKey();
		~_HandKey();

		virtual bool init(const json& j);
		virtual bool start(void);
		virtual void draw(void *pFrame);
		virtual bool check(void);

	private:
		void detect(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_HandKey *)This)->update();
			return NULL;
		}

	protected:
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
