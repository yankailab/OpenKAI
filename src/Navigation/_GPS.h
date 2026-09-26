/*
 * _GPS.h
 *
 *  Created on: Feb 23, 2019
 *      Author: yankai
 */

#ifndef OpenKAI_src_Navigation__GPS_H_
#define OpenKAI_src_Navigation__GPS_H_

#include "../IO/_IObase.h"
#include "../Dependencies/minmea.h"

namespace kai
{

	class _GPS : public _ModuleBase
	{
	public:
		_GPS(void);
		virtual ~_GPS();

		virtual bool loadConfig(void) override;
		virtual bool link(void) override;
		virtual bool start(void);
		virtual void console(void *pConsole);

	private:
		bool readNMEA(void);
		void decodeNMEA(void);
		virtual void update(void);
		static void *getUpdate(void *This)
		{
			((_GPS *)This)->update();
			return NULL;
		}

	protected:
		_IObase *m_pIO = nullptr;
		uint8_t m_rBuf[IO_BUF_N];
		int m_nRead = 0;
		int m_iRead = 0;
		string m_msg = "";

		minmea_sentence_rmc m_rmc;
		minmea_sentence_gga m_gga;
		minmea_sentence_gst m_gst;
		minmea_sentence_gsv m_gsv;
		minmea_sentence_vtg m_vtg;
		minmea_sentence_zda m_zda;

	};

}
#endif
