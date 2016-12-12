/*
 * IO.h
 *
 *  Created on: Dec 7, 2016
 *      Author: Kai Yan
 */

#ifndef OPENKAI_SRC_IO_IO_H_
#define OPENKAI_SRC_IO_IO_H_

#include "../Base/common.h"
#include "../Base/BASE.h"

namespace kai
{

enum IO_TYPE
{
	none,
	serialport,
	file,
	tcp,
	udp
};

enum IO_STATUS
{
	unknown,
	closed,
	opening
};

class IO: public BASE
{
public:
	IO();
	virtual ~IO();

	//unified interface
	virtual bool init(void* pKiss);
	virtual bool open(void);
	virtual bool isOpen(void);
	virtual void close(void);

	virtual int  read(uint8_t* pBuf, int nByte);
	virtual bool write(uint8_t* pBuf, int nByte);
	virtual bool writeLine(uint8_t* pBuf, int nByte);

	virtual IO_TYPE type(void);

public:
	IO_TYPE		m_type;
	IO_STATUS	m_status;
};

}

#endif
