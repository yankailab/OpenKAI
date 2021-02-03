/*
 * _RStracking.h
 *
 *  Created on: Oct 29, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_RStracking_H_
#define OpenKAI_src_Vision_RStracking_H_

#include "_SlamBase.h"

#ifdef USE_REALSENSE
#include <librealsense2/rs.hpp>

namespace kai
{

class _RStracking: public _SlamBase
{
public:
	_RStracking();
	virtual ~_RStracking();

	bool init(void* pKiss);
	bool start(void);
	void draw(void);
	bool open(void);
	void close(void);

private:
    void update(void);
	static void* getUpdate(void* This)
	{
		((_RStracking *) This)->update();
		return NULL;
	}

public:
    rs2::pipeline m_rsPipe;

};

}
#endif
#endif
