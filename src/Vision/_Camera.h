/*
 * _Camera.h
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Vision__Camera_H_
#define OpenKAI_src_Vision__Camera_H_

#include "_RGBbase.h"

namespace kai
{

	class _Camera : public _RGBbase
	{
	public:
		_Camera();
		virtual ~_Camera();

		virtual bool loadConfig(void) override;
		virtual bool saveConfig(bool bExport) override;
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

	protected:
		int m_deviceID = 0;
		VideoCapture m_camera;
	};

}
#endif
