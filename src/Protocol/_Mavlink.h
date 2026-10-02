#ifndef OpenKAI_src_Protocol__Mavlink_H_
#define OpenKAI_src_Protocol__Mavlink_H_

#include "../DataObject/MavlinkStream.h"
#include "../DataObject/BytePacketStream.h"

#include "../Base/_ModuleBase.h"
#include "../UI/_Console.h"
#include <atomic>
#include <mutex>

namespace kai
{
	class _Mavlink : public _ModuleBase
	{
	public:
		_Mavlink();
		~_Mavlink();

		virtual bool loadConfig(void) override;
		virtual bool saveConfig(bool bExport) override;
		virtual bool link(InstanceMgr *pM) override;
		virtual bool start(void);
		virtual bool check(void);
		virtual void console(void *pConsole);

	protected:
		bool writeMessage(const mavlink_message_t &msg);
		bool readMessage(mavlink_message_t *pMsg);

	private:
		void updateW(void);
		static void *getUpdateW(void *This)
		{
			((_Mavlink *)This)->updateW();
			return NULL;
		}

		void updateR(void);
		static void *getUpdateR(void *This)
		{
			((_Mavlink *)This)->updateR();
			return NULL;
		}

	protected:
		MavlinkStream *m_pMavStreamIn = nullptr;	// Messages queued for encoding and transmission to IO
		uint64_t m_tLastMavStreamIn = 0;

		_Thread *m_pTr = nullptr;					// Receive/decode worker; m_pT sends queued messages.
		MavlinkStream *m_pMavStreamOut = nullptr;	// Decoded messages received from IO

		BytePacketStream *m_pBpStreamIn = nullptr;	// StreamIn: byte packets received from IO
		uint64_t m_tLastBpStreamIn = 0;
		vector<BYTE_PACKET> m_vPacketIn;
		size_t m_iPacketIn = 0;
		size_t m_iByteIn = 0;
		uint8_t m_iMavComm = MAVLINK_COMM_0;		// Mavlink decode channel index
		mavlink_status_t m_status{};
		std::atomic<uint16_t> m_nDroppedPackets{0};

		BytePacketStream *m_pBpStreamOut = nullptr;	// StreamOut: byte packets to be written to IO

		int m_mySystemID = 255;
		int m_myComponentID = MAV_COMP_ID_MISSIONPLANNER;
		int m_myType = MAV_TYPE_GCS;
		int m_devSystemID = -1;
		int m_devComponentID = -1;
		int m_devType = 0;
	};

}
#endif
