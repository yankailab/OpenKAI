/*
 * _GPS.cpp
 *
 *  Created on: Feb 23, 2019
 *      Author: yankai
 */

#include "_GPS.h"

namespace kai
{

_GPS::_GPS()
{
	m_pIO = NULL;
	m_nRead = 0;
	m_iRead = 0;
	m_msg = "";

	m_LL.init();
	m_UTM.init();
}

_GPS::~_GPS()
{
}

bool _GPS::init(void* pKiss)
{
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string n = "";
	F_ERROR_F(pK->v("_IOBase", &n));
	m_pIO = (_IOBase*) (pK->getInst(n));
	IF_Fl(!m_pIO, "_IOBase not found");

	return true;
}

bool _GPS::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdate, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _GPS::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		if (readNMEA())
		{
			decodeNMEA();
			m_msg = "";
		}

		m_pT->autoFPSto();
	}
}

bool _GPS::readNMEA(void)
{
	if (m_nRead == 0)
	{
		m_nRead = m_pIO->read(m_rBuf, N_IO_BUF);
		IF_F(m_nRead <= 0);
		m_iRead = 0;
	}

	while (m_iRead < m_nRead)
	{
		char c = (char) m_rBuf[m_iRead];
		m_iRead++;

		if (m_msg.length() == 0)
		{
			if (c == '$' || c == '!')
			{
				m_msg += c;
			}
			continue;
		}

		const char pCRLF[] = "\x0d\x0a";
		IF_T(c == pCRLF[0]);

		m_msg += c;
	}

	m_nRead = 0;
	return false;
}

void _GPS::decodeNMEA(void)
{
		switch (minmea_sentence_id(m_msg.c_str(), false))
		{
		case MINMEA_SENTENCE_RMC:
		{
			if (minmea_parse_rmc(&m_rmc, m_msg.c_str()))
			{
				m_LL.m_lat = minmea_tocoord(&m_rmc.latitude);
				m_LL.m_lng = minmea_tocoord(&m_rmc.longitude);
				m_UTM = m_coord.LL2UTM(m_LL);

				LOG_I("$xxRMC: raw coordinates and speed: lat="
						+ i2str(m_rmc.latitude.value)
						+ "/"  + i2str(m_rmc.latitude.scale)
						+ ", lon=" + i2str(m_rmc.longitude.value)
						+ "/" + i2str(m_rmc.longitude.scale)
						+ ", spd=" + i2str(m_rmc.speed.value)
						+ "/" + i2str(m_rmc.speed.scale));

				LOG_I("$xxRMC fixed-point coordinates and speed scaled to three decimal places: lat="
						+ i2str(minmea_rescale(&m_rmc.latitude, 1000))
						+ ", lon=" + i2str(minmea_rescale(&m_rmc.longitude, 1000))
						+ ", spd=" + i2str(minmea_rescale(&m_rmc.speed, 1000)));

				LOG_I("$xxRMC floating point degree coordinates and speed: lat="
						+ f2str(minmea_tocoord(&m_rmc.latitude))
						+ ", lon=" + f2str(minmea_tocoord(&m_rmc.longitude))
						+ ", spd=" + f2str(minmea_tofloat(&m_rmc.speed)));
			}
			else
			{
				LOG_E("Sentence not parsed: " + m_msg);
			}
		}
			break;

		case MINMEA_SENTENCE_GGA:
		{
			if (minmea_parse_gga(&m_gga, m_msg.c_str()))
			{
				LOG_I("$xxGGA: fix quality: " + i2str(m_gga.fix_quality));
			}
			else
			{
				LOG_E("Sentence not parsed: " + m_msg);
			}
		}
			break;

		case MINMEA_SENTENCE_GST:
		{
			if (minmea_parse_gst(&m_gst, m_msg.c_str()))
			{
				LOG_I("$xxGST: raw latitude,longitude and altitude error deviation: "
						+ i2str(m_gst.latitude_error_deviation.value)
						+ i2str(m_gst.latitude_error_deviation.scale)
						+ i2str(m_gst.longitude_error_deviation.value)
						+ i2str(m_gst.longitude_error_deviation.scale)
						+ i2str(m_gst.altitude_error_deviation.value)
						+ i2str(m_gst.altitude_error_deviation.scale));

				LOG_I("$xxGST fixed point latitude,longitude and altitude error deviation scaled to one decimal place: "
						+ i2str(minmea_rescale(&m_gst.latitude_error_deviation, 10))
						+ i2str(minmea_rescale(&m_gst.longitude_error_deviation, 10))
						+ i2str(minmea_rescale(&m_gst.altitude_error_deviation, 10)));

				LOG_I("$xxGST floating point degree latitude, longitude and altitude error deviation: "
						+ f2str(minmea_tofloat(&m_gst.latitude_error_deviation))
						+ f2str(minmea_tofloat(&m_gst.longitude_error_deviation))
						+ f2str(minmea_tofloat(&m_gst.altitude_error_deviation)));
			}
			else
			{
				LOG_E("Sentence not parsed: " + m_msg);
			}
		}
			break;

		case MINMEA_SENTENCE_GSV:
		{
			if (minmea_parse_gsv(&m_gsv, m_msg.c_str()))
			{
				LOG_I("$xxGSV: message: " + i2str(m_gsv.msg_nr) + "/" + i2str(m_gsv.total_msgs));
				LOG_I("$xxGSV: sattelites in view: " + i2str(m_gsv.total_sats));

				for (int i = 0; i < 4; i++)
				{
					LOG_I("$xxGSV: sat nr " + i2str(m_gsv.sats[i].nr)
							+ ", elevation: " + i2str(m_gsv.sats[i].elevation)
							+ ", azimuth: " + i2str(m_gsv.sats[i].azimuth)
							+ ", snr: " + i2str(m_gsv.sats[i].snr)
							+ "dbm");
				}
			}
			else
			{
				LOG_E("Sentence not parsed: " + m_msg);
			}
		}
			break;

		case MINMEA_SENTENCE_VTG:
		{
			if (minmea_parse_vtg(&m_vtg, m_msg.c_str()))
			{
				LOG_I("$xxVTG: true track degrees = " + f2str(minmea_tofloat(&m_vtg.true_track_degrees)));
				LOG_I("magnetic track degrees = " + f2str(minmea_tofloat(&m_vtg.magnetic_track_degrees)));
				LOG_I("speed knots = " + f2str(minmea_tofloat(&m_vtg.speed_knots)));
				LOG_I("speed kph = " + f2str(minmea_tofloat(&m_vtg.speed_kph)));
			}
			else
			{
				LOG_E("Sentence not parsed: " + m_msg);
			}
		}
			break;

		case MINMEA_SENTENCE_ZDA:
		{
			if (minmea_parse_zda(&m_zda, m_msg.c_str()))
			{
				LOG_I("$xxZDA: "+i2str(m_zda.time.hours)
						+":"+i2str(m_zda.time.minutes)
						+":"+i2str(m_zda.time.seconds)
						+" "+i2str(m_zda.date.day)
						+"."+i2str(m_zda.date.month)
						+"."+i2str(m_zda.date.year)
						+" UTC%+"+i2str(m_zda.hour_offset)+":"
						+i2str(m_zda.minute_offset));
			}
			else
			{
				LOG_E("Sentence not parsed: " + m_msg);
			}
		}
			break;

		case MINMEA_INVALID:
		{
			LOG_E("Invalid sentence: " + m_msg);
		}
			break;

		default:
		{
			LOG_E("Sentence not parsed: " + m_msg);
		}
			break;
		}
}

LL_POS _GPS::getLLpos(void)
{
	return m_LL;
}

UTM_POS _GPS::getUTMpos(void)
{
	return m_UTM;
}

void _GPS::draw(void)
{
	this->_ModuleBase::draw();

	if(!m_pIO->isOpen())
	{
		addMsg("Not connected");
		return;
	}

	//RMC
	addMsg("RMC");
	addMsg("Raw: lat="
			+ i2str(m_rmc.latitude.value)
			+ "/"  + i2str(m_rmc.latitude.scale)
			+ ", lon=" + i2str(m_rmc.longitude.value)
			+ "/" + i2str(m_rmc.longitude.scale)
			+ ", spd=" + i2str(m_rmc.speed.value)
			+ "/" + i2str(m_rmc.speed.scale));

	addMsg("Fixed-point: lat="
			+ i2str(minmea_rescale(&m_rmc.latitude, 1000))
			+ ", lon=" + i2str(minmea_rescale(&m_rmc.longitude, 1000))
			+ ", spd=" + i2str(minmea_rescale(&m_rmc.speed, 1000)));

	addMsg("Floating-point: lat="
			+ f2str(minmea_tocoord(&m_rmc.latitude))
			+ ", lon=" + f2str(minmea_tocoord(&m_rmc.longitude))
			+ ", spd=" + f2str(minmea_tofloat(&m_rmc.speed)));

	//GGA
	addMsg("GGA");
	addMsg("Fix quality: " + i2str(m_gga.fix_quality));

	//GST
	addMsg("GST");
	addMsg("Raw latErr="
			+ i2str(m_gst.latitude_error_deviation.value) + "/"
			+ i2str(m_gst.latitude_error_deviation.scale) + ", lonErr="
			+ i2str(m_gst.longitude_error_deviation.value) + "/"
			+ i2str(m_gst.longitude_error_deviation.scale) + ", altErr="
			+ i2str(m_gst.altitude_error_deviation.value) + "/"
			+ i2str(m_gst.altitude_error_deviation.scale));

	addMsg("Fixed-point: latErr="
			+ i2str(minmea_rescale(&m_gst.latitude_error_deviation, 10)) + ", lonErr="
			+ i2str(minmea_rescale(&m_gst.longitude_error_deviation, 10)) + ", altErr="
			+ i2str(minmea_rescale(&m_gst.altitude_error_deviation, 10)));

	addMsg("Floating-point: latErr="
			+ f2str(minmea_tofloat(&m_gst.latitude_error_deviation)) + ", lonErr="
			+ f2str(minmea_tofloat(&m_gst.longitude_error_deviation)) + ", altErr="
			+ f2str(minmea_tofloat(&m_gst.altitude_error_deviation)));

	//GSV
	addMsg("GSV");
	addMsg("nMessage: " + i2str(m_gsv.msg_nr) + "/" + i2str(m_gsv.total_msgs));
	addMsg("nSattelites: " + i2str(m_gsv.total_sats));

	//VTG
	addMsg("VTG");
	addMsg("True track degrees: " + f2str(minmea_tofloat(&m_vtg.true_track_degrees)));
	addMsg("Magnetic track degrees: " + f2str(minmea_tofloat(&m_vtg.magnetic_track_degrees)));
	addMsg("Speed knots: " + f2str(minmea_tofloat(&m_vtg.speed_knots)));
	addMsg("speed kph: " + f2str(minmea_tofloat(&m_vtg.speed_kph)));

	//ZDA
	addMsg("ZDA");
	addMsg(""+i2str(m_zda.time.hours)
			+":"+i2str(m_zda.time.minutes)
			+":"+i2str(m_zda.time.seconds)
			+" "+i2str(m_zda.date.day)
			+"."+i2str(m_zda.date.month)
			+"."+i2str(m_zda.date.year)
			+" UTC%+"+i2str(m_zda.hour_offset)+":"
			+i2str(m_zda.minute_offset));

}

}
