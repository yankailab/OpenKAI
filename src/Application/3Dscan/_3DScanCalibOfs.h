/*
 * _3DScanCalibOfs.h
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_3Dscan__3DScanCalibOfs_H_
#define OpenKAI_src_Application_3Dscan__3DScanCalibOfs_H_

#include "../../Base/_ModuleBase.h"
#include "../../Vision/_VisionBase.h"

namespace kai
{
	class _3DScanCalibOfs : public _ModuleBase
	{
	public:
		_3DScanCalibOfs(void);
		virtual ~_3DScanCalibOfs();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		void draw(void *pFrame);

		// callbacks
		static void sOnBtnClear(void *pInst, uint32_t f);
		static void sOnBtnSave(void *pInst, uint32_t f);
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
			((_3DScanCalibOfs *)This)->update();
			return NULL;
		}

		void drawCalibData(Mat *pM);

	private:
		_VisionBase *m_pV;
//		_WindowCV *m_pW;

		float m_step;

		// draw
		Scalar m_drawCol;
//		cv::Ptr<freetype::FreeType2> m_pFt;
	};

}
#endif
