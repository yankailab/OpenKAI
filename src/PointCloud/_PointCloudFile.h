/*
 * _PointCloudFile.h
 *
 *  Created on: Sept 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_PointCloudFile_H_
#define OpenKAI_src_Vision_PointCloudFile_H_

#include "../Base/common.h"

#ifdef USE_OPENCV
#ifdef USE_OPEN3D
#include "_PointCloudViewer.h"

namespace kai
{

class _PointCloudFile: public _PointCloudBase
{
public:
	_PointCloudFile();
	virtual ~_PointCloudFile();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);
	int check(void);
	void draw(void);

private:
	void updateFile(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_PointCloudFile *) This)->update();
		return NULL;
	}

public:
	_PointCloudViewer* m_pViewer;
	string m_fName;

};

}
#endif
#endif
#endif
