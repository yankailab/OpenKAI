/*
 * _VzensePC.h
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_LIDAR_VzensePC_H_
#define OpenKAI_src_LIDAR_VzensePC_H_

#include "../3D/PointCloud/_PCframe.h"
#include "../Utility/util.h"
#include "../IPC/_SharedMem.h"
#include <VzenseNebula_api.h>

namespace kai
{

	class _VzensePC : public _PCframe
	{
	public:
		_VzensePC();
		virtual ~_VzensePC();

		virtual bool init(void *pKiss);
		virtual bool start(void);
		virtual int check(void);
		virtual bool open(void);
		virtual void close(void);

	private:
		bool updateVzense(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_VzensePC *)This)->update();
			return NULL;
		}

	public:
		uint32_t m_nDevice;
		VzDeviceInfo *m_pDeviceListInfo;
		string m_deviceURI;
		VzDeviceHandle m_deviceHandle;
        VzSensorIntrinsicParameters m_cameraParameters;

		bool m_bOpen;
		vInt2 m_vSize;
		uint16_t m_tWait;

		bool m_btDepth;
		bool m_btRGB;
		bool m_bIR;

        VzFrame m_vzfRGB;
        VzFrame m_vzfDepth;
        VzFrame m_vzfIR;

//		_SharedMem* m_psmTransformedDepth;

		VzVector3f* m_pVzVw; // world vector
		vFloat2 m_vRz; //z region
	};

}
#endif
