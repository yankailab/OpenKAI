/*
 * _GDcam.h
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Application__GDcam_H_
#define OpenKAI_src_Application__GDcam_H_

#include "../../Base/common.h"
#include "../../Base/_ThreadBase.h"
#include "../../Detector/_DetectorBase.h"
#include "../../Script/JSON.h"

namespace kai
{

class _GDcam: public _ThreadBase
{
public:
	_GDcam(void);
	virtual ~_GDcam();

	bool init(void* pKiss);
	bool start(void);
	bool draw(void);
	bool console(int& iY);
	int check(void);

private:
	bool findTarget(void);
	void updateShot(void);
	void oAlpr(const string& fImg);
	void gdUpload(const string& fImg);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_GDcam *) This)->update();
		return NULL;
	}

private:
	_DetectorBase* m_pD;
	Frame m_fBGR;
	int	m_iClass;

	bool m_bAlpr;
	bool m_bGDupload;

	vector<int> m_vJPGquality;
	string m_imgFile;
	string m_alprAPI;
	string m_alprKey;
	string m_gdUpload;
	string m_gdFolderID;

};

}
#endif
