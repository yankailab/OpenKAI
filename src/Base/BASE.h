/*
 * BASE.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_BASE_BASE_H_
#define OPENKAI_SRC_BASE_BASE_H_

#include "common.h"
#include "../Script/Config.h"

using namespace std;

namespace kai
{


class BASE
{
public:
	BASE();
	virtual ~BASE();

	virtual bool init(Config* pConfig);
	string* getName(void);
	string* getClass(void);

public:
	Config*	m_pConfig;
};

}

#endif /* OPENKAI_SRC_BASE_BASE_H_ */
