/*
 * Message.h
 *
 *  Created on: Dec 6, 2016
 *      Author: root
 */

#ifndef OPENKAI_SRC_BASE_MESSAGE_H_
#define OPENKAI_SRC_BASE_MESSAGE_H_

#include "common.h"
#include "../Utility/util.h"

#define MSG_BUF 512

//msg definitions
#define KMSG_HEADER_LEN 9
#define KMSG_BEGIN 0xff

#define KMSG_REQ_DATA 1
#define KMSG_SEND_DATA 2

/*
 * msg format
 * begin mark	[0]
 * cmd			[1~4]
 * payload byte	[5~8]
 * payload		[9~]
 */

/*
 * payload format
 * instTo length			[1]
 * (optional) instTo		[0~]
 * instFrom length			[1]
 * (optional) instFrom		[0~]
 * cmd dependent contents	[0~]
 * checksum					[4]
 * timeStamp				[8]
 */

namespace kai
{

class Message
{
public:
	Message();
	virtual ~Message();

	void init(void);
	int handle(uint8_t* pBuf, uint32_t nByte);
	bool encode(uint32_t cmd, string* pInstTo, string* pInstFrom, string* pData);

private:
	bool decode(void);

public:
	//header and raw contents
	uint32_t	m_cmd;
	uint32_t	m_nPayload;
	uint8_t*	m_pPayload;
	uint32_t	m_checksum;

	//cmd dependent
	string		m_instTo;
	string		m_instFrom;
	uint8_t*	m_pData;
	uint32_t	m_nData;

	//network info
	void*		m_pPeer;
	string		m_addrTo;
	string		m_addrFrom;

	//operation
	uint64_t	m_timeStamp;
	uint32_t	m_iByte;
	uint8_t		m_pBuf[MSG_BUF];


};

} /* namespace kai */

#endif /* OPENKAI_SRC_BASE_MESSAGE_H_ */
