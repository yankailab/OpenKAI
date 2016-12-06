/*
 * Message.cpp
 *
 *  Created on: Dec 6, 2016
 *      Author: root
 */

#include "Message.h"

namespace kai
{

Message::Message()
{
	init();
}

Message::~Message()
{
}

void Message::init(void)
{
	m_cmd = 0;
	m_nPayload = 0;
	m_pPayload = NULL;
	m_checksum = 0;
	m_timeStamp = 0;
	m_iByte = 0;
	m_pPeer = NULL;
	m_instTo = "";
	m_instFrom = "";
	m_pData = NULL;
	m_nData = 0;
}

int Message::handle(uint8_t* pBuf, uint32_t nByte)
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
			m_pBuf[m_iByte] = inByte;
			m_iByte++;

			//decode msg header
			if (m_iByte == 9)
			{
				m_cmd = makeUINT32(&m_pBuf[1]);
				m_nPayload = makeUINT32(&m_pBuf[5]);
			}
			else if (m_iByte == KMSG_HEADER_LEN + m_nPayload)
			{
				//decode the command
				if (decode())
					nDecoded++;
				m_iByte = 0;
			}
		}
		else if (inByte == KMSG_BEGIN)
		{
			m_pBuf[0] = inByte;
			m_nPayload = 0;
			m_iByte++;
		}

		//corrupted msg
		if (m_iByte >= MSG_BUF)
		{
			m_iByte = 0;
		}
	}

	return nDecoded;
}

bool Message::decode(void)
{
	uint8_t nInstTo;
	uint8_t nInstFrom;

	nInstTo = m_pPayload[0];
	m_instTo = "";
	if (nInstTo > 0)
	{
		m_instTo.assign((char*) &m_pPayload[1], nInstTo);
	}

	nInstFrom = m_pPayload[nInstTo + 1];
	m_instFrom = "";
	if (nInstFrom > 0)
	{
		m_instFrom.assign((char*) &m_pPayload[2 + nInstTo], nInstFrom);
	}

	uint32_t iData = nInstTo + nInstFrom + 2;
	m_pData = &m_pPayload[iData];
	m_nData = m_nPayload - iData;

	return true;
}

bool Message::encode(uint32_t cmd, string* pInstTo, string* pInstFrom,
		string* pData)
{
	m_instTo = "";
	m_instFrom = "";
	m_pData = NULL;
	m_nData = 0;

	m_cmd = cmd;
	if (pInstTo)
		m_instTo = *pInstTo;
	if (pInstFrom)
		m_instFrom = *pInstFrom;
	if (pData)
	{
		m_pData = (uint8_t*)pData->c_str();
		m_nData = pData->length();
	}

	m_nPayload = m_instTo.length() + m_instFrom.length() + 14 + m_nData;

	//encode msg buf
	m_iByte = 0;
	m_pBuf[0] = KMSG_BEGIN;

	copyByte(cmd, &m_pBuf[1]);
	copyByte(m_nPayload, &m_pBuf[5]);

	m_iByte = 9;
	m_pBuf[m_iByte++] = m_instTo.length();
	m_instTo.copy((char*) &m_pBuf[m_iByte], m_instTo.length());
	m_iByte += m_instTo.length();

	m_pBuf[m_iByte++] = m_instFrom.length();
	m_instFrom.copy((char*) &m_pBuf[m_iByte], m_instFrom.length());
	m_iByte += m_instFrom.length();

	pData->copy((char*) &m_pBuf[m_iByte], pData->length());
	m_iByte += pData->length();

	return true;
}

} /* namespace kai */
