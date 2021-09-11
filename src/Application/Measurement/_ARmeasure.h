/*
 * _ARmeasure.h
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_Measurement__ARarea_H_
#define OpenKAI_src_Application_Measurement__ARarea_H_

#include "../../Vision/_VisionBase.h"
#include "../../Navigation/_NavBase.h"
#include "../../Sensor/_DistSensorBase.h"

namespace kai
{
	enum ARmeasure_Mode
	{
		ARmeasure_vertex = 0,
		ARmeasure_free = 1,
		ARmeasure_calib = 2,
	};

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

	class _ARmeasure : public _ModuleBase
	{
	public:
		_ARmeasure(void);
		virtual ~_ARmeasure();

		bool init(void *pKiss);
		bool start(void);
		int check(void);
		void console(void *pConsole);
		void cvDraw(void *pWindow);

		// callbacks
		static void sOnBtnAction(void *pInst, uint32_t f);
		static void sOnBtnSave(void *pInst, uint32_t f);
		static void sOnBtnClear(void *pInst, uint32_t f);
		static void sOnBtnMode(void *pInst, uint32_t f);

		void action(void);
		void actionVertexMode(void);
		void actionFreeMode(void);
		void actionCablibMode(void);
		void save(void);
		void clear(void);
		void mode(uint32_t f);

	protected:
		void updateVertexMode(void);
		void updateFreeMode(void);
		void updateCalibMode(void);
		bool updateSensor(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_ARmeasure *)This)->update();
			return NULL;
		}

		bool updateBatt(void);
		void updateSlow(void);
		static void *getUpdateSlow(void *This)
		{
			((_ARmeasure *)This)->updateSlow();
			return NULL;
		}

		bool c2scr(const Vector3f &vPc,
				   const cv::Size &vSize,
				   const vFloat2 &vF,
				   const vFloat2 &vC,
				   cv::Point *pvPs);
		bool bInsideScr(const cv::Size &s, const cv::Point &p);

		void drawCross(Mat* pM);
		void drawVertices(Mat* pM);
		void drawMeasure(Mat* pM);
		void drawLidarRead(Mat* pM);
		void drawMsg(Mat* pM);
		void drawBatt(Mat* pM);

	private:
		_VisionBase *m_pV;
		_DistSensorBase *m_pD;
		_NavBase *m_pN;
		_WindowCV *m_pW;

		ARmeasure_Mode m_mode;
		bool m_bStarted;
		bool m_bSave;

		// result
		float m_area;
		float m_Dtot;

		// vertices
		vector<ARAREA_VERTEX> m_vVert;

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
		vFloat3 m_vDorgP; // distance sensor offset in pose sensor coordinate
		Vector3f m_vDptW; // point where the distance sensor is pointing at in world coordinate

		// camera
		vFloat3 m_vCorgP; // camera offset in pose sensor coordinate
		Eigen::Affine3f m_aW2C;
		vInt3 m_vAxisIdx;

		// draw
		string m_drawMsg;
		vFloat2 m_vCircleSize;
		int m_crossSize;
		Scalar m_drawCol;
		cv::Ptr<freetype::FreeType2> m_pFt;
		
		// slow jobs, system check etc.
	    _Thread* m_pTs;
		string m_cmdBatt;
		float m_battV;	// voltage
		float m_battA;	// current
		float m_battW;	// power
		float m_battP;	// percent
		float m_battShutdown;

	};

}
#endif
