/*
 * _RTCM3.h
 *
 *  Created on: Jun 3, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation__RTCM3_H_
#define OpenKAI_src_Navigation__RTCM3_H_

#include "../IO/_IOBase.h"
#include "../Dependency/rtklib/rtklib.h"

namespace kai
{

class _RTCM3: public _ThreadBase
{
public:
	_RTCM3(void);
	virtual ~_RTCM3();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);

private:
	void decode(void);
	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_RTCM3 *) This)->update();
		return NULL;
	}

private:
	_IOBase* m_pIO;
	uint8_t m_rBuf[N_IO_BUF];
	int m_nRead;
	int m_iRead;
	string m_msg;

};

}
#endif
