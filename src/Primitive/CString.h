/*
 * CString.h
 *
 *  Created on: June 25, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Primitive_CString_H_
#define OpenKAI_src_Primitive_CString_H_

#include "../Base/common.h"

#define CSTRING_N_BUF 256

namespace kai
{

class CString
{
public:
	CString();
	virtual ~CString();

	void reset(void);

public:
	char m_string[CSTRING_N_BUF];

};

}
#endif
