/*
 * _GDcam.h
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application_Surveillance__GDcam_H_
#define OpenKAI_src_Application_Surveillance__GDcam_H_

#ifdef USE_OPENCV
#include "../../Base/_ModuleBase.h"
#include "../../Detector/_DetectorBase.h"
#include "../../Script/JSON.h"

#define GD_N_CLASS 64

namespace kai
{

class _GDcam: public _ModuleBase
{
public:
	_GDcam(void);
	virtual ~_GDcam();

	bool init(void* pKiss);
	bool start(void);
	void draw(void* pFrame);
	int check(void);

private:
	bool findTarget(void);
	void updateShot(void);
	void oAlpr(const string& fName);
	void gdUpload(const string& fName);
	void update(void);
	static void* getUpdate(void* This)
	{
		((_GDcam *) This)->update();
		return NULL;
	}

private:
	_DetectorBase* m_pD;
	Frame m_fBGR;
	uint64_t m_classFlag;
	vFloat4 m_vRoi;
	bool m_bTarget;

	bool m_bAlpr;
	bool m_bGDupload;

	vector<int> m_vJPGquality;
	string m_tempDir;
	string m_alprAPI;
	string m_alprKey;
	fstream m_fAlpr;
	string m_gdUpload;
	string m_gdImgFolderID;
	string m_gdDataFolderID;
	string m_gdCredentials;

};

}
#endif
#endif
