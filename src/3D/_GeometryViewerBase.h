/*
 * _GeometryViewerBase.h
 *
 *  Created on: May 28, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D__GeometryViewerBase_H_
#define OpenKAI_src_3D__GeometryViewerBase_H_

#include "_GeometryBase.h"

namespace kai
{
	struct GVIEWER_CAM_PROJ
	{
		int m_type = 0; // 0:Perspective, 1:Ortho
		float m_fov = 70.0;

		// rendering boundaries in Ortho mode
		vFloat2 m_vLR = {-10, 10};
		vFloat2 m_vBT = {-10, 10};
		vFloat2 m_vNF = {0, FLT_MAX}; // near & far plane
	};

	struct GVIEWER_CAM_POSE
	{
		vFloat3 m_vLookAt = {0, 0, 0};
		vFloat3 m_vEye = {0, 0, 1};
		vFloat3 m_vUp = {0, 1, 0};
	};

	class _GeometryViewerBase : public _GeometryBase
	{
	public:
		_GeometryViewerBase();
		virtual ~_GeometryViewerBase();

		virtual bool init(const json &j);
		virtual bool link(const json &j, ModuleMgr *pM);
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
		vInt2 m_vWinSize;
		bool m_bFullScreen;
		string m_dirSave;

		GVIEWER_CAM_PROJ m_camProj;
		GVIEWER_CAM_POSE m_camPose, m_camPoseDefault;
		vFloat3 m_vCoR; // center of rotation

		// instance and buffer
		vector<_GeometryBase *> m_vpGb;
		GEOMETRY_RINGBUF<GEOMETRY_POINT> m_grPt;
        GEOMETRY_RINGBUF<GEOMETRY_LINE> m_grLn;
		int m_nPbuf;
		int m_nLbuf;
		uint64_t m_dTexpire;
	};

}
#endif
