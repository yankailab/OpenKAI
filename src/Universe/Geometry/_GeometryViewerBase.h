/*
 * _GeometryViewerBase.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry__GeometryViewerBase_H_
#define OpenKAI_src_Universe_Geometry__GeometryViewerBase_H_

#include "_GeometryBase.h"

namespace kai
{
	struct GVIEWER_CAM_PROJ
	{
		int m_type = 0; // 0:Perspective, 1:Ortho
		float m_fov = 70.0;

		// rendering boundaries in Ortho mode
		Vector2f m_vLR = {-10, 10};
		Vector2f m_vBT = {-10, 10};
		Vector2f m_vNF = {0, FLT_MAX}; // near & far plane
	};

	struct GVIEWER_CAM_POSE
	{
		Vector3f m_vLookAt = {0, 0, 0};
		Vector3f m_vEye = {0, 0, 1};
		Vector3f m_vUp = {0, 1, 0};
	};

	class _GeometryViewerBase : public _GeometryBase
	{
	public:
		_GeometryViewerBase();
		virtual ~_GeometryViewerBase();

		virtual bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);

		virtual void resetCamPose(void);
		virtual void setCamPose(const GVIEWER_CAM_POSE &camPose);
		virtual GVIEWER_CAM_POSE getCamPose(void);
		virtual void setCamProj(const GVIEWER_CAM_PROJ &camProj);
		virtual GVIEWER_CAM_PROJ getCamProj(void);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_GeometryViewerBase *)This)->update();
			return NULL;
		}

	protected:
		virtual void updateAllGeometries(void);
		virtual void updateCamProj(void);
		virtual void updateCamPose(void);

	protected:
		// rendering
		Vector2i m_vWinSize = Vector2i::Zero();
		bool m_bFullScreen = false;
		string m_dirSave = "";

		GVIEWER_CAM_PROJ m_camProj;
		GVIEWER_CAM_POSE m_camPose, m_camPoseDefault;
		Vector3f m_vCoR = Vector3f::Zero(); // center of rotation

		// instance and buffer
		GEOMETRY_RINGBUF<GEOMETRY_POINT> m_grPt;
        GEOMETRY_RINGBUF<GEOMETRY_LINE> m_grLn;
		bool m_bGeometryBuffers = true;
		int m_nPbuf = 200000;
		int m_nLbuf = 100000;
		uint64_t m_dTexpire = 0; // nanoseconds; zero disables expiry
	};

}
#endif
