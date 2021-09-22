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
		static void sOnBtnAction(void *pInst, uint32_t f);
		static void sOnBtnClear(void *pInst, uint32_t f);
		static void sOnBtnSave(void *pInst, uint32_t f);
		static void sOnBtnMode(void *pInst, uint32_t f);
		static void sOnBtnL(void *pInst, uint32_t f);
		static void sOnBtnR(void *pInst, uint32_t f);
		static void sOnBtnU(void *pInst, uint32_t f);
		static void sOnBtnD(void *pInst, uint32_t f);
		static void sOnBtnF(void *pInst, uint32_t f);
		static void sOnBtnB(void *pInst, uint32_t f);
		static void sOnBtnSl(void *pInst, uint32_t f);
		static void sOnBtnSs(void *pInst, uint32_t f);

		void action(void);
		void clear(void);
		void save(void);
		void mode(uint32_t f);
		void L(void);
		void R(void);
		void U(void);
		void D(void);
		void F(void);
		void B(void);
		void Sl(void);
		void Ss(void);

	protected:
		void updateCalib(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ARmeasureCalibDofs *)This)->update();
			return NULL;
		}

	    bool saveCalib(void);
		void drawCalibData(Mat *pM);
		void drawMsg(Mat *pM);

	private:
		_ARmeasure* m_pA;
		_VisionBase *m_pV;
		_WindowCV *m_pW;

		string m_fKiss;
		float m_step;

		// draw
		string m_drawMsg;
		Scalar m_drawCol;
		cv::Ptr<freetype::FreeType2> m_pFt;
	};

}
#endif
