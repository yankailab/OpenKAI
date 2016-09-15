/*
 * BASE.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_BASE_BASE_H_
#define OPENKAI_SRC_BASE_BASE_H_

#include "platform.h"

using namespace std;

namespace kai
{


class BASE
{
public:
	BASE();
	virtual ~BASE();

public:
	string m_name;
};

}

#endif /* OPENKAI_SRC_BASE_BASE_H_ */
