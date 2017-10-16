/*
 * _DataBase.h
 *
 *  Created on: Oct 16, 2017
 *      Author: yankai
 */

#ifndef OpenKAI_src_Data__DataBase_H_
#define OpenKAI_src_Data__DataBase_H_

#include "../Base/common.h"
#include "../Base/_ThreadBase.h"
#include "../Utility/util.h"

namespace kai
{

class _DataBase: public _ThreadBase
{
public:
	_DataBase();
	~_DataBase();

	bool init(void* pKiss);
	bool link(void);
	bool start(void);

private:
	void getInputFileList(void);
	void openOutput(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_DataBase*) This)->update();
		return NULL;
	}

public:
	string m_dirIn;
	string m_dirOut;
	string m_extIn;
	string m_extOut;

	vector<string> m_vFileIn;


};
}

#endif
