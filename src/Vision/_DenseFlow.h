/*
 * _DenseFlow.h
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Flow_H_
#define OpenKAI_src_Vision__Flow_H_

#include "../Base/common.h"
#include "_VisionBase.h"

namespace kai
{

	class _DenseFlow : public _ModuleBase
	{
	public:
		_DenseFlow();
		virtual ~_DenseFlow();

		virtual bool init(const json& j);
		virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual void draw(void *pFrame);

		vDouble2 vFlow(vInt4 *pROI);
		vDouble2 vFlow(vDouble4 *pROI);

	private:
		bool isFlowCorrect(Point2f u);
		Vec3b computeColor(float fx, float fy);
		void drawOpticalFlow(const Mat_<float> &flowx, const Mat_<float> &flowy, Mat &dst, float maxmotion);
		void detect(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_DenseFlow *)This)->update();
			return NULL;
		}

	protected:
		int m_w;
		int m_h;
		_VisionBase *m_pV;
//		FrameGroup *m_pGrayFrames;
		Ptr<cuda::FarnebackOpticalFlow> m_pFarn;
		GpuMat m_gFlow;
		Mat m_pFlow[2];

		int m_nHistLev;
		vDouble2 m_vRange;
		double m_minHistD;
	};

}
#endif
