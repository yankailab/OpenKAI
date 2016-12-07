/*
 * Interface.h
 *
 *  Created on: Dec 7, 2016
 *      Author: root
 */

#ifndef OPENKAI_SRC_INTERFACE_INTERFACE_H_
#define OPENKAI_SRC_INTERFACE_INTERFACE_H_

#include "../Base/common.h"
#include "../IO/SerialPort.h"
#include "../IO/FileIO.h"
#include "../Network/_server.h"

namespace kai
{

enum IF_TYPE
{
	none,
	uart,
	file,
	network
};

enum IF_STATUS
{
	unknown,
	disconnected,
	connected
};

class _Interface: public _ThreadBase
{
public:
	_Interface();
	virtual ~_Interface();

	//unified interface
	bool init(void* pKiss);
	bool link(void);
	bool start(void);

	int  read(void);

private:
	bool connect(void);
	bool connectSerialPort(void);
	bool connectFile(void);
	bool connectServer(void);

	void update(void);
	static void* getUpdateThread(void* This)
	{
		((_Interface*) This)->update();
		return NULL;
	}

public:
	IF_TYPE		m_type;
	IF_STATUS	m_status;

	//serial port
	SerialPort* m_pSerialPort;
	string*		m_pSportName;
	int 		m_baudRate;

	//network
	_server*	m_pServer;
	_peer*		m_pPeer;
	Message*	m_pMsg;

	//file
	FileIO* 	m_pFile;
	string*		m_pFileName;

private:
	//internal buffer
	uint32_t	m_nBuf;
	uint8_t*	m_pBuf;
	string		m_strBuf;


};

} /* namespace kai */

#endif /* OPENKAI_SRC_INTERFACE_INTERFACE_H_ */
