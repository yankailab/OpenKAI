/*
 * BASE.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_BASE_BASE_H_
#define OPENKAI_SRC_BASE_BASE_H_

#include "../Script/Kiss.h"
#include "common.h"

using namespace std;

namespace kai
{


class BASE
{
public:
	BASE();
	virtual ~BASE();

	virtual bool init(Kiss* pKiss);
	string* getName(void);
	string* getClass(void);

public:
	Kiss*	m_pKiss;
};

}

#endif /* OPENKAI_SRC_BASE_BASE_H_ */
