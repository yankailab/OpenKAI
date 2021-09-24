/*
 * _ARmeasureCalibDofs.h
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_Measurement__ARmeasureCalibDofs_H_
#define OpenKAI_src_Application_Measurement__ARmeasureCalibDofs_H_

#include "_ARmeasure.h"

namespace kai
{
	class _ARmeasureCalibDofs : public _StateBase
	{
	public:
		_ARmeasureCalibDofs(void);
		virtual ~_ARmeasureCalibDofs();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		void cvDraw(void *pWindow);

		// callbacks
		static void sOnBtnClear(void *pInst, uint32_t f);
		static void sOnBtnSave(void *pInst, uint32_t f);
		static void sOnBtnMode(void *pInst, uint32_t f);
		static void sOnBtnXi(void *pInst, uint32_t f);
		static void sOnBtnXd(void *pInst, uint32_t f);
		static void sOnBtnYi(void *pInst, uint32_t f);
		static void sOnBtnYd(void *pInst, uint32_t f);
		static void sOnBtnZi(void *pInst, uint32_t f);
		static void sOnBtnZd(void *pInst, uint32_t f);
		static void sOnBtnSi(void *pInst, uint32_t f);
		static void sOnBtnSd(void *pInst, uint32_t f);

		void clear(void);
		void save(void);
		void mode(uint32_t f);
		void Xi(void);
		void Xd(void);
		void Yi(void);
		void Yd(void);
		void Zi(void);
		void Zd(void);
		void Si(void);
		void Sd(void);

	protected:
		void updateCalib(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ARmeasureCalibDofs *)This)->update();
			return NULL;
		}

		void drawCalibData(Mat *pM);
		void drawMsg(Mat *pM);

	private:
		_ARmeasure* m_pA;
		_VisionBase *m_pV;
		_WindowCV *m_pW;

		float m_step;

		// draw
		string m_drawMsg;
		Scalar m_drawCol;
		cv::Ptr<freetype::FreeType2> m_pFt;
	};

}
#endif
