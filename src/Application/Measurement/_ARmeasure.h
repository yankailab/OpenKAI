/*
 * _ARmeasure.h
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_Measurement__ARmeasure_H_
#define OpenKAI_src_Application_Measurement__ARmeasure_H_

#include "../../State/_StateBase.h"
#include "../../Vision/_VisionBase.h"
#include "../../Navigation/_NavBase.h"
#include "../../Sensor/_DistSensorBase.h"
#include "../../Utility/utilFile.h"

namespace kai
{
	const string ARmeasureModeLabel[] =
	{
		"V","F","Calib",
	};

	struct ARAREA_VERTEX
	{
		Vector3f m_vVertW; //world coordinate
		cv::Point m_vPs;   //screen
		bool m_bZ;		   //if inside the projection plane
	};

	class _ARmeasure : public _StateBase
	{
	public:
		_ARmeasure(void);
		virtual ~_ARmeasure();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		void console(void *pConsole);
		void cvDraw(void *pWindow);

		bool c2scr(const Vector3f &vPc,
				   const cv::Size &vSize,
				   const vFloat2 &vF,
				   const vFloat2 &vC,
				   cv::Point *pvPs);
		bool bInsideScr(const cv::Size &s, const cv::Point &p);
		bool bValid(void);
		Vector3f vDptW(void);
		Eigen::Affine3f aW2C(void);

		vFloat3 getDofsP(void);
		vFloat3 getCofsP(void);
		void setDofsP(const vFloat3& v);
		void setCofsP(const vFloat3& v);

		bool saveCalib(void);

		// callbacks
		static void sOnBtnSave(void *pInst, uint32_t f);
		void save(void);

	protected:
		bool updateSensor(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ARmeasure *)This)->update();
			return NULL;
		}

		void drawCross(Mat* pM);
		void drawLidarRead(Mat* pM);
		void drawMsg(Mat* pM);

	private:
		_VisionBase *m_pV;
		_DistSensorBase *m_pD;
		_NavBase *m_pN;
		_WindowCV *m_pW;

		bool m_bSave;
		string m_fCalibOfs;

		// pose
		Eigen::Affine3f m_aPose;
		Vector3f m_vDptP;
		float m_minPoseConfidence;

		// distance sensor
		float m_d;
		bool m_bValidDist;
		bool m_bValidPose;
		vFloat2 m_vKlaserSpot; // laser spot size over distance
		vFloat2 m_vRange; // dist measured out this range is ignored as invalid
		vFloat3 m_vDofsP; // distance sensor offset in pose sensor coordinate
		Vector3f m_vDptW; // point where the distance sensor is pointing at in world coordinate

		// camera
		vFloat3 m_vCofsP; // camera offset in pose sensor coordinate
		Eigen::Affine3f m_aW2C;
		vInt3 m_vAxisIdx;

		// draw
		string m_drawMsg;
		vFloat2 m_vCircleSize;
		int m_crossSize;
		Scalar m_drawCol;
		cv::Ptr<freetype::FreeType2> m_pFt;		
	};

}
#endif
