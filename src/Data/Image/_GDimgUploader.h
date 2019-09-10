/*
 * _GDimgUploader.h
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__GDimgUploader_H_
#define OpenKAI_src_Application__GDimgUploader_H_

#include "../../Base/common.h"
#include "../../Detector/_DetectorBase.h"
#include "../../Script/JSON.h"

namespace kai
{

class _GDimgUploader: public _ThreadBase
{
public:
	_GDimgUploader(void);
	virtual ~_GDimgUploader();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

private:
	void updateUpload(void);
	void gdUpload(OBJECT* pO);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_GDimgUploader *) This)->update();
		return NULL;
	}

private:
	_DetectorBase* m_pD;

	vector<int> m_vJPGquality;
	string m_tempDir;
	fstream m_fMeta;
	string m_gdUpload;
	string m_gdImgFolderID;
	string m_gdDataFolderID;
	string m_gdCredentials;

};

}
#endif
