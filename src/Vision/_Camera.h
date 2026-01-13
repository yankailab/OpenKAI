/*
 * _Camera.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Camera_H_
#define OpenKAI_src_Vision__Camera_H_

#include "_VisionBase.h"

namespace kai
{

	class _Camera : public _VisionBase
	{
	public:
		_Camera();
		virtual ~_Camera();

		virtual bool init(const json& j);
		virtual bool start(void);
		bool open(void);
		void close(void);

	private:
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_Camera *)This)->update();
			return NULL;
		}

	public:
		int m_deviceID;
		VideoCapture m_camera;
		int m_nInitRead;
		bool m_bResetCam;
	};

}
#endif
