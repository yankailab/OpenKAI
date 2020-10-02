/*
 * _PCtransform.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCtransform_H_
#define OpenKAI_src_PointCloud_PCtransform_H_

#include "../Base/common.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D
#include "_PointCloudBase.h"

namespace kai
{

class _PCtransform: public _PointCloudBase
{
public:
	_PCtransform();
	virtual ~_PCtransform();

	bool init(void* pKiss);
	bool start(void);
	int check(void);

	virtual void setTranslation(vFloat3& vT);
	virtual void setRotation(vFloat3& vR);
	virtual void setTranslationMatrix(int i, Eigen::Matrix4d_u& mR);

private:
	void updateTransform(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PCtransform *) This)->update();
		return NULL;
	}

public:
	vFloat3 m_vT;	//translation
	vFloat3 m_vR;	//rotation
	vector<Eigen::Matrix4d> m_vmT; //additional transform matrices

};

}
#endif
#endif
#endif
