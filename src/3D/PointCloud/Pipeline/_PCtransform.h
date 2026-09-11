/*
 * _PCtransform.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_PCtransform_H_
#define OpenKAI_src_3D_PointCloud_PCtransform_H_

#include "../_PointCloud.h"
#include "../../../Utility/utilFile.h"
#include "../../../IO/_File.h"

namespace kai
{

	class _PCtransform : public _PointCloud
	{
	public:
		_PCtransform();
		virtual ~_PCtransform();

		virtual bool init(const json &j);
        virtual bool link(const json &j, ModuleMgr *pM);
		virtual bool start(void);
		virtual bool check(void);
        virtual void console(void *pConsole);

		virtual bool loadConfig(json *pJ = nullptr, string fName = "");
		virtual bool saveConfig(json &j, string fName = "");

		virtual void clear(void);

		// attitude
		virtual void setTranslation(const vDouble3 &vT);
		virtual void setRotation(const vDouble3 &vR);
		virtual void setQuaternion(const vDouble4 &vQ);
		virtual void updateTranslationMatrix(bool bUseQuaternion = true, vDouble3 *pRa = NULL);
		virtual Matrix4d createTranslationMatrix(const vDouble3 &vT, const vDouble3 &vR, vDouble3 *pRa = NULL);
		virtual Matrix4d createTranslationMatrix(const vDouble3 &vT, const vDouble4 &vQ, vDouble3 *pRa = NULL);
		virtual void setTranslationMatrix(const Matrix4d &mT);

		virtual Matrix4d getTranslationMatrix(void);
		virtual vDouble3 getTranslation(void);
		virtual vDouble3 getRotation(void);
		virtual vDouble4 getQuaternion(void);

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
        vDouble3 m_vT;
        vDouble3 m_vR;
        vDouble4 m_vQ;
        Matrix4d m_mT;
        Eigen::Affine3d m_A;

	};

}
#endif
