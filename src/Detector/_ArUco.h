/*
 * _ArUco.h
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__ArUco_H_
#define OpenKAI_src_Detector__ArUco_H_

#include "../Detector/_DetectorBase.h"

namespace kai
{

	class _ArUco : public _DetectorBase
	{
	public:
		_ArUco();
		virtual ~_ArUco();

		bool init(void *pKiss);
		bool start(void);
		void draw(void *pFrame);
		void console(void *pConsole);
		int check(void);

	private:
		void detect(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ArUco *)This)->update();
			return NULL;
		}

	public:
		cv::Ptr<cv::aruco::Dictionary> m_pDict;
		uint8_t m_dict;
		float m_realSize;

		// optional camera matrix
		bool m_bPose;
		Mat m_mC;		// Intrinsic
		Mat m_mCscaled; // scaled with input image size
		Mat m_mD;		// Distortion
	};

}
#endif
