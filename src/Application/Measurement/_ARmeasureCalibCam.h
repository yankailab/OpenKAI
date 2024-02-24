/*
 * _ARmeasureCalibCam.h
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_Measurement__ARmeasureCalibCam_H_
#define OpenKAI_src_Application_Measurement__ARmeasureCalibCam_H_

#include "_ARmeasure.h"
#include "../../Vision/ImgFilter/_Remap.h"

namespace kai
{
	class _ARmeasureCalibCam : public _ModuleBase
	{
	public:
		_ARmeasureCalibCam(void);
		virtual ~_ARmeasureCalibCam();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		void draw(void *pFrame);

		// callbacks
		static void sOnBtnAction(void *pInst, uint32_t f);
		static void sOnBtnClear(void *pInst, uint32_t f);
		static void sOnBtnSave(void *pInst, uint32_t f);
		static void sOnBtnMode(void *pInst, uint32_t f);

		void action(void);
		void clear(void);
		void save(uint32_t f);
		void mode(uint32_t f);

	protected:
		void updateCalib(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ARmeasureCalibCam *)This)->update();
			return NULL;
		}

	    bool camCalib(void);
	    bool saveCalib(void);
		void drawCalibData(Mat *pM);

	private:
		_ARmeasure* m_pA;
		_Remap *m_pR;
		_WindowCV *m_pW;

		// camera
		vector<Point3f> m_vPo;			// world coordinates for 3D points
		vector<vector<Point3f>> m_vvPpo; // 3D points for each checkerboard image
		vector<vector<Point2f>> m_vvPimg; // 2D points for each checkerboard image

		Mat m_mCalib;	// for calib preview
		int m_iPreview;
		int m_tPreview;
		vInt2 m_vChessBoardSize;	// col, row
		float m_squareSize;
		Mat m_mC;
		Mat m_mD;
		string m_fCalib;

		// draw
		Scalar m_drawCol;
		cv::Ptr<freetype::FreeType2> m_pFt;
	};

}
#endif
