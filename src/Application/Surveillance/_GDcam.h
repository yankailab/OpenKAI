/*
 * _GDcam.h
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__GDcam_H_
#define OpenKAI_src_Application__GDcam_H_

#include "../../Base/_ThreadBase.h"
#include "../../Detector/_DetectorBase.h"
#include "../../Script/JSON.h"

#ifdef USE_OPENCV

#define GD_N_CLASS 64

namespace kai
{

class _GDcam: public _ThreadBase
{
public:
	_GDcam(void);
	virtual ~_GDcam();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	int check(void);

private:
	bool findTarget(void);
	void updateShot(void);
	void oAlpr(const string& fName);
	void gdUpload(const string& fName);
	void update(void);
	static void* getUpdateThread(void* This)
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
