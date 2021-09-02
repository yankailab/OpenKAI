/*
 * _PCrs.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifdef USE_OPEN3D
#ifdef USE_REALSENSE
#include "_PCrs.h"

namespace kai
{

	_PCrs::_PCrs()
	{
		m_rsSN = "";
		m_vPreset = "Short Range";
		m_bRsRGB = true;
		m_rsFPS = 30;
		m_rsDFPS = 30;
		m_fDec = 0.0;
		m_fSpat = 0.0;
		m_bAlign = false;
		m_rspAlign = NULL;
		m_fEmitter = 1.0;
		m_fLaserPower = 1.0;

		m_bOpen = false;
		m_vWHc.init(1280, 720);
		m_vWHd.init(640, 480);
		m_vRz.init(0.0, FLT_MAX);
	}

	_PCrs::~_PCrs()
	{
		DEL(m_rspAlign);
	}

	bool _PCrs::init(void *pKiss)
	{
		IF_F(!_PCframe::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("rsSN", &m_rsSN);
		pK->v("rsFPS", &m_rsFPS);
		pK->v("rsDFPS", &m_rsDFPS);
		pK->v("vPreset", &m_vPreset);
		pK->v("bRsRGB", &m_bRsRGB);
		pK->v("fDec", &m_fDec);
		pK->v("fSpat", &m_fSpat);
		pK->v("bAlign", &m_bAlign);
		pK->v("fEmitter", &m_fEmitter);
		pK->v("fLaserPower", &m_fLaserPower);

		pK->v("vRz", &m_vRz);
		pK->v("vWHcol", &m_vWHc);
		pK->v("vWHd", &m_vWHd);

		return true;
	}

	bool _PCrs::open(void)
	{
		IF_T(m_bOpen);

		try
		{
			if (!m_rsSN.empty())
				m_rsConfig.enable_device(m_rsSN);

			m_rsConfig.enable_stream(RS2_STREAM_DEPTH, m_vWHd.x, m_vWHd.y, RS2_FORMAT_Z16, m_rsDFPS);
			if (m_bRsRGB)
				m_rsConfig.enable_stream(RS2_STREAM_COLOR, m_vWHc.x, m_vWHc.y, RS2_FORMAT_BGR8, m_rsFPS);

			m_rsProfile = m_rsPipe.start(m_rsConfig);
			rs2::device dev = m_rsProfile.get_device();
			LOG_I("Device Name:" + string(dev.get_info(RS2_CAMERA_INFO_NAME)));
			LOG_I("Firmware Version:" + string(dev.get_info(RS2_CAMERA_INFO_FIRMWARE_VERSION)));
			LOG_I("Serial Number:" + string(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)));
			LOG_I("Product Id:" + string(dev.get_info(RS2_CAMERA_INFO_PRODUCT_ID)));

			auto sensor = m_rsProfile.get_device().first<rs2::depth_sensor>();
			auto range = sensor.get_option_range(RS2_OPTION_VISUAL_PRESET);
			for (auto i = range.min; i <= range.max; i += range.step)
			{
				IF_CONT(std::string(sensor.get_option_value_description(RS2_OPTION_VISUAL_PRESET, i)) != m_vPreset);
				sensor.set_option(RS2_OPTION_VISUAL_PRESET, i);
				break;
			}

			if (m_fDec > 0.0)
				m_rsfDec.set_option(RS2_OPTION_FILTER_MAGNITUDE, m_fDec);
			if (m_fSpat > 0.0)
				m_rsfSpat.set_option(RS2_OPTION_HOLES_FILL, m_fSpat);

			if (sensor.supports(RS2_OPTION_EMITTER_ENABLED))
				sensor.set_option(RS2_OPTION_EMITTER_ENABLED, m_fEmitter);

			if (sensor.supports(RS2_OPTION_LASER_POWER))
			{
				auto range = sensor.get_option_range(RS2_OPTION_LASER_POWER);
				sensor.set_option(RS2_OPTION_LASER_POWER, range.max * m_fLaserPower);
			}

			rs2::frameset rsFrameset = m_rsPipe.wait_for_frames();

			if (m_bRsRGB)
			{
				if (m_bAlign)
				{
					m_rspAlign = new rs2::align(rs2_stream::RS2_STREAM_COLOR);
					rs2::frameset rsFramesetAlign = m_rspAlign->process(rsFrameset);
					m_rsColor = rsFramesetAlign.get_color_frame();
					m_rsDepth = rsFramesetAlign.get_depth_frame();
				}
				else
				{
					m_rsColor = rsFrameset.get_color_frame();
					m_rsDepth = rsFrameset.get_depth_frame();
				}

				m_vWHc.x = m_rsColor.as<rs2::video_frame>().get_width();
				m_vWHc.y = m_rsColor.as<rs2::video_frame>().get_height();
			}
			else
			{
				m_rsDepth = rsFrameset.get_depth_frame();
			}

			if (m_fDec > 0.0)
				m_rsDepth = m_rsfDec.process(m_rsDepth);
			if (m_fSpat > 0.0)
				m_rsDepth = m_rsfSpat.process(m_rsDepth);

			m_vWHd.x = m_rsDepth.as<rs2::video_frame>().get_width();
			m_vWHd.y = m_rsDepth.as<rs2::video_frame>().get_height();
		}
		catch (const rs2::camera_disconnected_error &e)
		{
			LOG_E("Realsense disconnected");
			return false;
		}
		catch (const rs2::recoverable_error &e)
		{
			LOG_E("Realsense open failed");
			return false;
		}
		catch (const rs2::error &e)
		{
			LOG_E("Realsense error");
			return false;
		}
		catch (const std::exception &e)
		{
			LOG_E("Realsense exception");
			return false;
		}

		m_spImg = std::make_shared<geometry::Image>();
		m_spImg->Prepare(m_vWHc.x, m_vWHc.y, 3, 1);

		m_bOpen = true;
		return true;
	}

	void _PCrs::sensorReset(void)
	{
		//    m_rsConfig.resolve(m_rsPipe).get_device().hardware_reset();

		rs2::device dev = m_rsProfile.get_device();
		dev.hardware_reset();
	}

	bool _PCrs::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _PCrs::check(void)
	{
		return this->_PCframe::check();
	}

	void _PCrs::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					LOG_E("Cannot open RealSense");
					sensorReset();
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPSfrom();

			if (updateRS())
			{
				updatePC();
			}
			else
			{
				sensorReset();
				m_pT->sleepT(SEC_2_USEC);
				m_bOpen = false;
			}

			m_pT->autoFPSto();
		}
	}

	bool _PCrs::updateRS(void)
	{
		IF_T(check() < 0);

		try
		{
			rs2::frameset rsFrameset = m_rsPipe.wait_for_frames();

			if (m_bRsRGB)
			{
				if (m_bAlign)
				{
					rs2::frameset rsFramesetAlign = m_rspAlign->process(rsFrameset);
					m_rsColor = rsFramesetAlign.get_color_frame();
					m_rsDepth = rsFramesetAlign.get_depth_frame();
				}
				else
				{
					m_rsColor = rsFrameset.get_color_frame();
					m_rsDepth = rsFrameset.get_depth_frame();
				}
			}
			else
			{
				m_rsDepth = rsFrameset.get_depth_frame();
			}
		}
		catch (const rs2::camera_disconnected_error &e)
		{
			LOG_E("Realsense disconnected");
			return false;
		}
		catch (const rs2::recoverable_error &e)
		{
			LOG_E("Realsense open failed");
			return false;
		}
		catch (const rs2::error &e)
		{
			LOG_E("Realsense error");
			return false;
		}
		catch (const std::exception &e)
		{
			LOG_E("Realsense exception");
			return false;
		}

		m_rsPC.map_to(m_rsColor);
		m_rsPoints = m_rsPC.calculate(m_rsDepth);

		memcpy(m_spImg->data_.data(),
			   m_rsColor.get_data(),
			   m_vWHc.area() * 3);

		auto rspVertex = m_rsPoints.get_vertices();
		auto rspTexCoord = m_rsPoints.get_texture_coordinates();
		int nP = m_rsPoints.size();

		PointCloud *pPC = m_sPC.next();

		const static float c_b = 1.0 / 255.0;

		for (int i = 0; i < nP; i++)
		{
			rs2::vertex vr = rspVertex[i];
			IF_CONT(vr.z < m_vRz.x);
			IF_CONT(vr.z > m_vRz.y);

			Eigen::Vector3d ve(vr.x, vr.y, vr.z);
			pPC->points_.push_back(ve);

			rs2::texture_coordinate tc = rspTexCoord[i];
			int tx = constrain<int>(tc.u * m_vWHc.x, 0, m_vWHc.x - 1);
			int ty = constrain<int>(tc.v * m_vWHc.y, 0, m_vWHc.y - 1);
			Eigen::Vector3d te((double)*m_spImg->PointerAt<uint8_t>(tx, ty, 2),
							   (double)*m_spImg->PointerAt<uint8_t>(tx, ty, 1),
							   (double)*m_spImg->PointerAt<uint8_t>(tx, ty, 0));
			te *= c_b;
			pPC->colors_.push_back(te);
		}

		return true;
	}

}
#endif
#endif

//		auto cIntr = m_pRS->m_cIntrinsics;
//		auto dIntr = m_pRS->m_dIntrinsics;
//		dIntr = cIntr;
//		m_imgD.Prepare(dIntr.width, dIntr.height, 1, 2);
//		m_imgRGB.Prepare(cIntr.width, cIntr.height, 3, 1);
//		memcpy(m_imgD.data_.data(), m_pRS->m_rsDepth.get_data(), dIntr.width * dIntr.height * 2);
//		memcpy(m_imgRGB.data_.data(), m_pRS->m_rsColor.get_data(), cIntr.width * cIntr.height * 3);
//
//		shared_ptr<RGBDImage> imgRGBD = RGBDImage::CreateFromColorAndDepth(m_imgRGB, m_imgD, 1.0/m_pRS->m_dScale, m_pRS->m_vRange.y, false);
//        camera::PinholeCameraIntrinsic camInt(dIntr.width,
//        										dIntr.height,
//												dIntr.fx,
//												dIntr.fy,
//												dIntr.ppx,
//												dIntr.ppy);
//        m_spPC = PointCloud::CreateFromRGBDImage(*imgRGBD, camInt);
//        m_pPC = m_spPC;
