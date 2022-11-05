/*
 * _Vzense.h
 *
 *  Created on: Sept 7, 2022
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision_Vzense_H_
#define OpenKAI_src_Vision_Vzense_H_

#include "../Base/_ModuleBase.h"
#include "../Utility/util.h"
#include "../IPC/_SharedMem.h"
#include <VzenseNebula_api.h>

namespace kai
{

	class _Vzense : public _ModuleBase
	{
	public:
		_Vzense();
		virtual ~_Vzense();

		virtual bool init(void *pKiss);
		virtual bool link(void);
		virtual bool start(void);
		virtual int check(void);
		virtual bool open(void);
		virtual void close(void);

	private:
		void update(void);
		bool updateVzense(void);
		static void *getUpdate(void *This)
		{
			((_Vzense *)This)->update();
			return NULL;
		}

		void updateTPP(void);
		static void *getTPP(void *This)
		{
			((_Vzense *)This)->updateTPP();
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

		bool m_bDepth;
		bool m_bIR;
		bool m_bRGB;
		bool m_btDepth;
		bool m_btRGB;

        VzFrame m_vzfRGB;
        VzFrame m_vzfDepth;
        VzFrame m_vzfTransformedDepth;
        VzFrame m_vzfTransformedRGB;
        VzFrame m_vzfIR;

		_SharedMem* m_psmRGB;
		_SharedMem* m_psmDepth;
		_SharedMem* m_psmTransformedDepth;
		_SharedMem* m_psmTransformedRGB;
		_SharedMem* m_psmIR;


		float m_fConfidenceThreshold;

		//depth filter processing thread
		_Thread *m_pTPP;
	};

}
#endif
