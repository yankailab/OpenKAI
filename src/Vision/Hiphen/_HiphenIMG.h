/*
 * _HiphenIMG.h
 *
 *  	Created on: Feb 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_IO__HiphenIMG_H_
#define OpenKAI_src_IO__HiphenIMG_H_

#include "../../Base/common.h"
#include "../../IO/_TCPclient.h"

namespace kai
{

#define N_HIPHEN_BUF 3000000
#define N_HIPHEN_HEADER 136
#define N_HIPHEN_FILESIZE 8
#define N_HIPHEN_FILENAME 128

class _HiphenIMG: public _TCPclient
{
public:
	_HiphenIMG();
	virtual ~_HiphenIMG();

	bool init(void* pKiss);
	bool start(void);
	void decodeData(void);
	void decodeHeader(void);

	void updateR(void);
	static void* getUpdateThreadR(void* This)
	{
		((_HiphenIMG*) This)->updateR();
		return NULL;
	}

public:
	uint8_t m_pBuf[N_HIPHEN_BUF];
	int m_iB;
	int m_nB;
	int m_nbImg;

	string m_dir;
	string m_subDir;
	string m_fileName;
};

}
#endif
