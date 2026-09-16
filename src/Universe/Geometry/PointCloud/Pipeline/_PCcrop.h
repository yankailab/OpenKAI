/*
 * _PCcrop.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_PointCloud_PCcrop_H_
#define OpenKAI_src_Universe_Geometry_PointCloud_PCcrop_H_

#include "../../_GeometryBase.h"

namespace kai
{

	enum POINTCLOUD_VOL_TYPE
	{
		pc_vol_box = 0,
		pc_vol_ball = 1,
		pc_vol_cylinder = 2,
	};

	struct POINTCLOUD_VOL
	{
		POINTCLOUD_VOL_TYPE m_type;
		bool m_bInside; // true: inside valid

		Vector2f m_vX = Vector2f::Zero();
		Vector2f m_vY = Vector2f::Zero();
		Vector2f m_vZ = Vector2f::Zero();

		Vector3f m_vC = Vector3f::Zero();
		Vector2f m_vR = Vector2f::Zero();

		void init(void)
		{
			m_type = pc_vol_ball;
			m_bInside = false;
			m_vX.setZero();
			m_vY.setZero();
			m_vZ.setZero();
			m_vC.setZero();
			m_vR.setZero();
		}

		bool bValid(Vector3f &vP)
		{
			bool bInside = true;

			if (m_type == pc_vol_box)
			{
				if (vP.x() < m_vX.x())
					bInside = false;
				if (vP.x() > m_vX.y())
					bInside = false;
				if (vP.y() < m_vY.x())
					bInside = false;
				if (vP.y() > m_vY.y())
					bInside = false;
				if (vP.z() < m_vZ.x())
					bInside = false;
				if (vP.z() > m_vZ.y())
					bInside = false;
			}
			else if (m_type == pc_vol_ball)
			{
				Vector3f vR = vP - m_vC;
				float r = vR.norm();
				if (r < m_vR.x())
					bInside = false;
				if (r > m_vR.y())
					bInside = false;
			}
			else if (m_type == pc_vol_cylinder)
			{
				Vector3f vR = vP - m_vC;
				vR.z() = 0.0;
				float r = vR.norm();
				if (r < m_vR.x())
					bInside = false;
				if (r > m_vR.y())
					bInside = false;
			}

			IF_F(m_bInside != bInside);
			return true;
		}
	};

	class _PCcrop : public _GeometryBase
	{
	public:
		_PCcrop();
		virtual ~_PCcrop();

		virtual bool init(const json &j);
		virtual bool start(void);
		virtual bool check(void);

	private:
		virtual void update(void);
		void updateFilter(void);
		virtual bool bFilter(Vector3d &vP);
		static void *getUpdate(void *This)
		{
			((_PCcrop *)This)->update();
			return NULL;
		}

	protected:
		vector<POINTCLOUD_VOL> m_vFilter;
	};

}
#endif
