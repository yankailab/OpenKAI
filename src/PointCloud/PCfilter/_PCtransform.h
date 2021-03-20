/*
 * _PCtransform.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud_PCtransform_H_
#define OpenKAI_src_PointCloud_PCtransform_H_
#ifdef USE_OPEN3D

#include "../_PCframe.h"
#include "../../IO/_File.h"

namespace kai
{

class _PCtransform: public _PCframe
{
public:
	_PCtransform();
	virtual ~_PCtransform();

	bool init(void* pKiss);
	bool start(void);
	int check(void);

	virtual void setTranslationMatrix(int i, Eigen::Matrix4d_u& mT);
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
	vector<Eigen::Matrix4d> m_vmT; //additional transform matrices
	string m_paramKiss;
};

}
#endif
#endif