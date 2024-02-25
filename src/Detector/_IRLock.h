/*
 * _IRLock.h
 *
 *  Created on: Nov 20, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Detector__IRLock_H_
#define OpenKAI_src_Detector__IRLock_H_

#include "../Detector/_DetectorBase.h"
#include "../IO/_IObase.h"

namespace kai
{

	/*
	 Bytes    16-bit words   Description
	----------------------------------------------------------------
	0, 1     0              sync (0xaa55)
	2, 3     1              checksum (sum of all 16-bit words 2-6)

	6, 7     2              signature number
	8, 9     3              x center of object
	10, 11   4              y center of object
	12, 13   5              width of object
	14, 15   6              height of object
	*/

#define IRLOCK_N_BUF 16
#define IRLOCK_N_BLOCK 16
#define IRLOCK_SYNC_L 0x55
#define IRLOCK_SYNC_H 0xaa

	class _IRLock : public _DetectorBase
	{
	public:
		_IRLock();
		virtual ~_IRLock();

		bool init(void *pKiss);
		bool link(void);
		bool start(void);
		void console(void *pConsole);
		int check(void);

	private:
		bool readPacket(void);
		void detect(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_IRLock *)This)->update();
			return NULL;
		}

	public:
		_IObase *m_pIO;
		uint8_t m_pBuf[IRLOCK_N_BUF];
		int m_iBuf;
		vFloat2 m_vOvCamSize;
	};

}
#endif
