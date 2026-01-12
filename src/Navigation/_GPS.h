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
#include "Coordinate.h"

namespace kai
{

	class _GPS : public _ModuleBase
	{
	public:
		_GPS(void);
		virtual ~_GPS();

		bool init(const json& j);
		bool start(void);
		void console(void *pConsole);

		LL_POS getLLpos(void);
		UTM_POS getUTMpos(void);

	private:
		bool readNMEA(void);
		void decodeNMEA(void);
		void update(void);
		static void *getUpdate(void *This)
		{
			((_GPS *)This)->update();
			return NULL;
		}

	private:
		_IObase *m_pIO;
		uint8_t m_rBuf[IO_BUF_N];
		int m_nRead;
		int m_iRead;
		string m_msg;

		minmea_sentence_rmc m_rmc;
		minmea_sentence_gga m_gga;
		minmea_sentence_gst m_gst;
		minmea_sentence_gsv m_gsv;
		minmea_sentence_vtg m_vtg;
		minmea_sentence_zda m_zda;

		LL_POS m_LL;
		UTM_POS m_UTM;
		Coordinate m_coord;
	};

}
#endif
