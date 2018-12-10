/*
 * Message.cpp
 *
 *  Created on: Dec 6, 2016
 *      Author: root
 */

#include "Peer.h"

namespace kai
{

Peer::Peer()
{
	init();
}

Peer::~Peer()
{
}

void Peer::init(void)
{
	m_timeStamp = 0;
	m_pPeer = NULL;

	reset();
}

void Peer::reset(void)
{
	m_nMsg = MSG_LEN_MAX;
	m_cmd = 0;
	m_nInstTo = 0;
	m_instTo = "";
	m_nInstFrom = 0;
	m_instFrom = "";
	m_checksum = 0;
	m_nPayload = 0;
	m_pPayload = NULL;

	m_iByte = 0;
}

int Peer::handle(uint8_t* pBuf, uint32_t nByte)
{
	if (nByte == 0)
		return -1;
	if (pBuf == NULL)
		return -1;

	uint8_t inByte;
	uint32_t i;
	int nDecoded = 0;

	for (i = 0; i < nByte; i++)
	{
		inByte = pBuf[i];

		if (m_iByte > 0)
		{
			//receiving a message
			m_pBuf[m_iByte] = inByte;
			m_iByte++;

			if (m_iByte == 5)
			{
//				m_nMsg = makeUINT32(&m_pBuf[1]);
				if (m_nMsg > MSG_LEN_MAX)
				{
					//invalid msg
					reset();
				}
			}
			else if (m_iByte == m_nMsg)
			{
				if(decode())nDecoded++;
				reset();
			}
		}
		else if (inByte == KMSG_BEGIN)
		{
			//found a new message
			m_pBuf[0] = inByte;
			m_nPayload = 0;
			m_iByte++;
		}

		//corrupted msg
		if (m_iByte >= MSG_LEN_MAX)
		{
			reset();
		}
	}

	return nDecoded;
}

bool Peer::decode(void)
{
	IF_F(m_iByte < m_nMsg);

	int iM = 5;
//	m_cmd = makeUINT32(&m_pBuf[iM]);
	iM += 4;

	m_nInstTo = m_pBuf[iM++];
	if (m_nInstTo > 0)
	{
		m_instTo.assign((char*) &m_pBuf[iM], m_nInstTo);
		iM += m_nInstTo;
	}

	m_nInstFrom = m_pBuf[iM++];
	if (m_nInstFrom > 0)
	{
		m_instFrom.assign((char*) &m_pBuf[iM], m_nInstFrom);
		iM += m_nInstFrom;
	}

//	m_checksum = makeUINT32(&m_pBuf[iM]);
	iM += 4;

//	m_nPayload = makeUINT32(&m_pBuf[iM]);
	iM += 4;
	m_pPayload = &m_pBuf[iM];

	return true;
}

bool Peer::encode(uint32_t cmd, string* pInstTo, string* pInstFrom,
		string* pPayload)
{
	reset();

	m_cmd = cmd;
	if (pInstTo)
	{
		m_instTo = *pInstTo;
		m_nInstTo = m_instTo.length();
	}
	if (pInstFrom)
	{
		m_instFrom = *pInstFrom;
		m_nInstFrom = m_instFrom.length();
	}
	if (pPayload)
	{
		m_pPayload = (uint8_t*) pPayload->c_str();
		m_nPayload = pPayload->length();
	}

	m_nMsg = 19 + m_nInstTo + m_nInstFrom + m_nPayload;

	//encode msg buf
	m_iByte = 0;
	m_pBuf[m_iByte++] = KMSG_BEGIN;

//	copyByte(m_nMsg, &m_pBuf[m_iByte]);
	m_iByte += 4;

//	copyByte(m_cmd, &m_pBuf[m_iByte]);
	m_iByte += 4;

	m_pBuf[m_iByte++] = m_nInstTo;
	if(m_nInstTo>0)
	{
		m_instTo.copy((char*) &m_pBuf[m_iByte], m_nInstTo);
		m_iByte += m_nInstTo;
	}

	m_pBuf[m_iByte++] = m_nInstFrom;
	if(m_nInstFrom>0)
	{
		m_instFrom.copy((char*) &m_pBuf[m_iByte], m_nInstFrom);
		m_iByte += m_nInstFrom;
	}

//	copyByte(m_checksum, &m_pBuf[m_iByte]);
	m_iByte += 4;

//	copyByte(m_nPayload, &m_pBuf[m_iByte]);
	m_iByte += 4;

	if(m_nPayload)
	{
		pPayload->copy((char*) &m_pBuf[m_iByte], m_nPayload);
		m_iByte += m_nPayload;
	}

	return true;
}

}
