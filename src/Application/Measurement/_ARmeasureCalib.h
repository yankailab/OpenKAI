/*
 * _ARmeasureCalib.h
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_Measurement__ARmeasureCalib_H_
#define OpenKAI_src_Application_Measurement__ARmeasureCalib_H_

#include "_ARmeasure.h"
#include "../../Vision/ImgFilter/_Remap.h"

namespace kai
{
	class _ARmeasureCalib : public _StateBase
	{
	public:
		_ARmeasureCalib(void);
		virtual ~_ARmeasureCalib();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		void cvDraw(void *pWindow);

		// callbacks
		static void sOnBtnAction(void *pInst, uint32_t f);
		static void sOnBtnClear(void *pInst, uint32_t f);
		static void sOnBtnMode(void *pInst, uint32_t f);

		void action(void);
		void clear(void);
		void mode(uint32_t f);

	protected:
		void updateCalib(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ARmeasureCalib *)This)->update();
			return NULL;
		}

	    bool camCalib(void);
	    bool saveCalib(void);

	private:
		_ARmeasure* m_pA;
		_Remap *m_pV;
		_WindowCV *m_pW;

		vector<Point3f> m_vPo;			// world coordinates for 3D points
		vector<Mat> m_vImg;
		vector<vector<Point3f>> m_vvPpo; // 3D points for each checkerboard image
		vector<vector<Point2f>> m_vvPimg; // 2D points for each checkerboard image

		vInt2 m_vChessBoardSize;	//col, row
		float m_squareSize;
		Mat m_mC;
		Mat m_mD;

		string m_fKiss;

		// draw
		Scalar m_drawCol;
		cv::Ptr<freetype::FreeType2> m_pFt;
	};

}
#endif
