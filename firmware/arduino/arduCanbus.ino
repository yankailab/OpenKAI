//Serial to Can-bus converter using Arduino + CAN-BUS Shield
#include <Arduino.h>
#include <mcp_can.h>
#include <SPI.h>

//Protocol
#define CMD_BUF_LEN 256
#define MAVLINK_BEGIN 0xFE
#define MAVLINK_HEADDER_LEN 3
//temporal
//0 START MARK
//1 PAYLOAD LENGTH
//2 COMMAND
//3 Payload...

//Commands
#define CMD_CAN_SEND 0
#define CMD_PIN_OUTPUT 2
#define CMD_OPERATE_MODE 1

//Can-bus shield
// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;
MCP_CAN CAN(SPI_CS_PIN);  // Set CS pin

struct CMD_STREAM
{
	uint8_t m_cmd;
	uint8_t m_payloadLen;
	uint16_t m_iByte;
	uint8_t m_pBuf[CMD_BUF_LEN];
};
CMD_STREAM m_cmd;

union longBytes
{
    byte m_pByte[4];
    long m_long;
};
longBytes lb;

void command(void)
{
	int i;
	unsigned long* addr;
	byte len;
	byte* pData;

	//Command
	switch (m_cmd.m_pBuf[2])
	{
	case CMD_CAN_SEND:
		addr = (unsigned long*)(&m_cmd.m_pBuf[3]);
		len = m_cmd.m_pBuf[7];
		pData = &m_cmd.m_pBuf[8];

		// send data:  id = 0x00, standrad frame, data len = 8, stmp: data buf
		CAN.sendMsgBuf(*addr, 0, len, pData);
		break;

	case CMD_PIN_OUTPUT:
		digitalWrite(m_cmd.m_pBuf[3], m_cmd.m_pBuf[4]);
		break;

	default:
		break;
	}
}

bool receive(void)
{
	byte	inByte;

	while (Serial.available() > 0)
	{
		inByte = Serial.read();

		if (m_cmd.m_cmd != 0)
		{
			m_cmd.m_pBuf[m_cmd.m_iByte] = inByte;
			m_cmd.m_iByte++;

			if (m_cmd.m_iByte == 2)	//Payload length
			{
				m_cmd.m_payloadLen = inByte;
			}
			else if (m_cmd.m_iByte == m_cmd.m_payloadLen + MAVLINK_HEADDER_LEN)
			{
				//decode the command
				command();
				m_cmd.m_cmd = 0;

				//Execute one command at a time
				return true;
			}
		}
		else if (inByte == MAVLINK_BEGIN)
		{
			m_cmd.m_cmd = inByte;
			m_cmd.m_pBuf[0] = inByte;
			m_cmd.m_iByte = 1;
			m_cmd.m_payloadLen = 0;
		}
	}

	return false;
}

void setup()
{
	pinMode(11,OUTPUT);
	pinMode(12,OUTPUT);
	pinMode(13,OUTPUT);

	Serial.begin(115200);

	while (CAN_OK != CAN.begin(CAN_500KBPS))  // init can bus : baudrate = 500k
	{
		Serial.println("CAN BUS Shield init fail\nInit CAN BUS Shield again");
		delay(100);
	}
	Serial.println("CAN BUS Shield init ok!");
}

void loop()
{
	receive();

    unsigned char len = 0;
    unsigned char buf[8];

    if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data coming
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
        lb.m_long = CAN.getCanId();

        if(lb.m_long == 769)
        {        
        Serial.write(MAVLINK_BEGIN);			//start mark
        Serial.write(13);						//payload len
        Serial.write((uint8_t)CMD_CAN_SEND);	//cmd
        Serial.write(lb.m_pByte[0]);
        Serial.write(lb.m_pByte[1]);
        Serial.write(lb.m_pByte[2]);
        Serial.write(lb.m_pByte[3]);
        Serial.write(len);						//len
        Serial.write(buf, 8);					//data
        }
    }
}



