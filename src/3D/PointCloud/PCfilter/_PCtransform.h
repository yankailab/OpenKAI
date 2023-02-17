/*
 * _PCtransform.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_3D_PointCloud_PCtransform_H_
#define OpenKAI_src_3D_PointCloud_PCtransform_H_

#include "../_PCframe.h"
#include "../../../IO/_File.h"

namespace kai
{

	class _PCtransform : public _PCframe
	{
	public:
		_PCtransform();
		virtual ~_PCtransform();

		bool init(void *pKiss);
		bool start(void);
		int check(void);

		virtual void setTranslationMatrix(Matrix4d_u &mTt);
		virtual void saveParamKiss(void);

	private:
		void updateTransform(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_PCtransform *)This)->update();
			return NULL;
		}

	public:
		Matrix4d_u m_mTt;
		string m_paramKiss;
	};

}
#endif
