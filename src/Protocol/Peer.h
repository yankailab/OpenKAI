/*
 * Peer.h
 *
 *  Created on: Dec 6, 2016
 *      Author: root
 */

#ifndef OPENKAI_SRC_PROTOCOL_PEER_H_
#define OPENKAI_SRC_PROTOCOL_PEER_H_

#include "../Base/common.h"
#include "../Utility/util.h"

#define MSG_LEN_MAX 512

//Msg header
#define KMSG_BEGIN 0xff
#define KMSG_HEADER_LEN

/*
 * msg format
 * begin mark		[1]
 * msg byte			[4]
 * cmd				[4]
 * instTo byte  	[1]
 * instTo			[0~]
 * instFrom byte	[1]
 * instFrom			[0~]
 * checksum			[4]
 * payload byte		[4]
 * payload			[0~]
 */

//Msg ref
#define KMSG_REQ_DATA 1
#define KMSG_SEND_DATA 2


namespace kai
{

class Peer
{
public:
	Peer();
	virtual ~Peer();

	void init(void);
	int handle(uint8_t* pBuf, uint32_t nByte);
	bool encode(uint32_t cmd, string* pInstTo, string* pInstFrom, string* pPayload);

private:
	bool decode(void);
	void reset(void);

public:
	//header and raw contents
	uint32_t	m_nMsg;
	uint32_t	m_cmd;
	uint8_t		m_nInstTo;
	string		m_instTo;
	uint8_t		m_nInstFrom;
	string		m_instFrom;
	uint32_t	m_checksum;
	uint32_t	m_nPayload;
	uint8_t*	m_pPayload;

	//network info
	void*		m_pPeer;
	string		m_addrTo;
	string		m_addrFrom;

	//operation
	uint64_t	m_timeStamp;
	uint32_t	m_iByte;
	uint8_t		m_pBuf[MSG_LEN_MAX];

};

} /* namespace kai */

#endif /* OPENKAI_SRC_BASE_MESSAGE_H_ */
