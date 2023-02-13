/*
 * _VzensePC.h
 *
 *  Created on: Feb 13, 2023
 *      Author: yankai
 */

#ifndef OpenKAI_src_LIDAR_VzensePC_H_
#define OpenKAI_src_LIDAR_VzensePC_H_

#include "../3D/PointCloud/_PCframe.h"

namespace kai
{

	class _VzensePC : public _PCframe
	{
	public:
		_VzensePC();
		virtual ~_VzensePC();

		bool init(void *pKiss);
		bool open(void);
		bool start(void);
		int check(void);

	private:
		void sensorReset(void);
		bool updateRS(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_VzensePC *)This)->update();
			return NULL;
		}

	public:
		string m_rsSN;
		float m_fFilterManitude;
		float m_fHolesFill;
		bool m_bAlign;
		float m_fEmitter;
		float m_fLaserPower;

		int m_rsFPS;
		int m_rsDFPS;
		bool m_bRsRGB;
		string m_vPreset;

		bool m_bOpen;
		vInt2 m_vWHc;
		vInt2 m_vWHd;
		shared_ptr<Image> m_spImg;
		vFloat2 m_vRz; //z region
	};

}
#endif
