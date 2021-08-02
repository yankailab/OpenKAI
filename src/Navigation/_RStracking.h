/*
 * _RStracking.h
 *
 *  Created on: Oct 29, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation_RStracking_H_
#define OpenKAI_src_Navigation_RStracking_H_

#ifdef USE_REALSENSE
#include "_NavBase.h"
#include <librealsense2/rs.hpp>

namespace kai
{

class _RStracking: public _NavBase
{
public:
	_RStracking();
	virtual ~_RStracking();

	bool init(void* pKiss);
	bool start(void);
	bool open(void);
	void close(void);

private:
	bool updateRS(void);
    void update(void);
	static void* getUpdate(void* This)
	{
		((_RStracking *) This)->update();
		return NULL;
	}

	void hardwareReset ( void );

public:
    rs2::pipeline m_rsPipe;

};

}
#endif
#endif
