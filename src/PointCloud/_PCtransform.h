/*
 * _PCtransform.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCtransform_H_
#define OpenKAI_src_PointCloud_PCtransform_H_

#include "../Base/common.h"
#include "../IO/_File.h"

#ifdef USE_OPEN3D
#include "_PCbase.h"

namespace kai
{

class _PCtransform: public _PCbase
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

	virtual vFloat3 getTranslation(void);
	virtual vFloat3 getRotation(void);
	virtual Eigen::Matrix4d getTranslationMatrix(int i);

	virtual void saveParamKiss(void);

private:
	void updateTransform(void);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_PCtransform *) This)->update();
		return NULL;
	}

public:
	vFloat3 m_vT;	//translation
	vFloat3 m_vR;	//rotation
	vector<Eigen::Matrix4d> m_vmT; //additional transform matrices

	string m_paramKiss;
};

}
#endif
#endif
