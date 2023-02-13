/*
 * _3DScanCalibCam.h
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_3Dscan__3DScanCalibCam_H_
#define OpenKAI_src_Application_3Dscan__3DScanCalibCam_H_

#include "../../Vision/ImgFilter/_Remap.h"
#include "../../Utility/utilEvent.h"

namespace kai
{
	class _3DScanCalibCam : public _ModuleBase
	{
	public:
		_3DScanCalibCam(void);
		virtual ~_3DScanCalibCam();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		void draw(void *pFrame);

		// callbacks
		static void sOnBtnAction(void *pInst, uint32_t f);
		static void sOnBtnClear(void *pInst, uint32_t f);
		static void sOnBtnSave(void *pInst, uint32_t f);

		void action(void);
		void clear(void);
		void save(uint32_t f);

	protected:
		void updateCalib(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_3DScanCalibCam *)This)->update();
			return NULL;
		}

	    bool camCalib(void);
	    bool saveCalib(void);
		void drawCalibData(Mat *pM);
		void drawMsg(Mat *pM);

	private:
		_Remap *m_pR;
//		_WindowCV *m_pW;

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
		MSG_SHOW m_msg;
		Scalar m_drawCol;
//		cv::Ptr<freetype::FreeType2> m_pFt;
	};

}
#endif
