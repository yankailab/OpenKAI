/*
 * _PCtransform.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Universe_Geometry_PointCloud_PCtransform_H_
#define OpenKAI_src_Universe_Geometry_PointCloud_PCtransform_H_

#include "../_PointCloud.h"
#include "../../../../Utility/utilFile.h"
#include "../../../../IO/_File.h"

namespace kai
{

	class _PCtransform : public _PointCloud
	{
	public:
		_PCtransform();
		virtual ~_PCtransform();

		virtual bool loadConfig(void) override;
        virtual bool link(void) override;
		virtual bool start(void);
		virtual bool check(void);
        virtual void console(void *pConsole);

		virtual bool saveConfig(void) override;

		virtual void clear(void);

		// attitude
		virtual void setTranslation(const Vector3d &vT);
		virtual void setRotation(const Vector3d &vR);
		virtual void setQuaternion(const Vector4d &vQ);
		virtual void updateTranslationMatrix(bool bUseQuaternion = true, Vector3d *pRa = NULL);
		virtual Eigen::Matrix4d createTranslationMatrix(const Vector3d &vT, const Vector3d &vR, Vector3d *pRa = NULL);
		virtual Eigen::Matrix4d createTranslationMatrix(const Vector3d &vT, const Vector4d &vQ, Vector3d *pRa = NULL);
		virtual void setTranslationMatrix(const Eigen::Matrix4d &mT);

		virtual Eigen::Matrix4d getTranslationMatrix(void);
		virtual Vector3d getTranslation(void);
		virtual Vector3d getRotation(void);
		virtual Vector4d getQuaternion(void);

	private:
		void updateTransform(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_PCtransform *)This)->update();
			return NULL;
		}

	protected:
		_PointCloud* m_pPS = nullptr;
		uint64_t m_dTexpire = 0;

        // transform
        Vector3d m_vT = Vector3d::Zero();
        Vector3d m_vR = Vector3d::Zero();
        Vector4d m_vQ = Vector4d::Zero();
        Eigen::Matrix4d m_mT;
        Eigen::Affine3d m_A;

	};

}
#endif
