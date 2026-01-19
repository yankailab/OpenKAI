/*
 * _RealSense.cpp
 *
 *  Created on: Apr 6, 2018
 *      Author: yankai
 */

#include "_RealSense.h"

namespace kai
{

    _RealSense::_RealSense()
    {
        m_rsSN = "";
        m_rsFPS = 30;
        m_rsDFPS = 30;
        m_bAlign = false;
        m_vPreset = "High Density";
        m_rspAlign = nullptr;
        m_rsCtrl.clear();

        m_vSizeRGB.set(1280, 720);
        m_vSizeD.set(640, 480);
    }

    _RealSense::~_RealSense()
    {
        DEL(m_pTpp);
        DEL(m_rspAlign);
    }

    bool _RealSense::init(const json &j)
    {
        IF_F(!_RGBDbase::init(j));

        jKv(j, "rsSN", m_rsSN);
        jKv(j, "rsFPS", m_rsFPS);
        jKv(j, "rsDFPS", m_rsDFPS);
        jKv(j, "bAlign", m_bAlign);
        jKv(j, "vPreset", m_vPreset);

        jKv(j, "fConfidenceThreshold", m_rsCtrl.m_fConfidenceThreshold);
        jKv(j, "fDigitalGain", m_rsCtrl.m_fDigitalGain);
        jKv(j, "fPostProcessingSharpening", m_rsCtrl.m_fPostProcessingSharpening);
        jKv(j, "fFilterMagnitude", m_rsCtrl.m_fFilterMagnitude);
        jKv(j, "fHolesFill", m_rsCtrl.m_fHolesFill);
        jKv(j, "fEmitter", m_rsCtrl.m_fEmitter);
        jKv(j, "fLaserPower", m_rsCtrl.m_fLaserPower);

        jKv(j, "fBrightness", m_rsCtrl.m_fBrightness);
        jKv(j, "fContrast", m_rsCtrl.m_fContrast);
        jKv(j, "fGain", m_rsCtrl.m_fGain);
        jKv(j, "fExposure", m_rsCtrl.m_fExposure);
        jKv(j, "fHue", m_rsCtrl.m_fHue);
        jKv(j, "fSaturation", m_rsCtrl.m_fSaturation);
        jKv(j, "fSharpness", m_rsCtrl.m_fSharpness);
        jKv(j, "fWhiteBalance", m_rsCtrl.m_fWhiteBalance);

        DEL(m_pTpp);
        m_pTpp = createThread(j.at("threadPP"), "threadPP");
        NULL_F(m_pTpp);

        return true;
    }

    bool _RealSense::open(void)
    {
        IF_F(m_bOpen);

        try
        {
            if (!m_rsSN.empty())
                m_rsConfig.enable_device(m_rsSN);

            m_rsConfig.enable_stream(RS2_STREAM_DEPTH, m_vSizeD.x, m_vSizeD.y, RS2_FORMAT_Z16, m_rsDFPS);
            if (m_bRGB)
                m_rsConfig.enable_stream(RS2_STREAM_COLOR, m_vSizeRGB.x, m_vSizeRGB.y, RS2_FORMAT_BGR8, m_rsFPS);

            m_rsProfile = m_rsPipe.start(m_rsConfig);
            rs2::device dev = m_rsProfile.get_device();
            LOG_I("Device Name:" + string(dev.get_info(RS2_CAMERA_INFO_NAME)));
            LOG_I("Firmware Version:" + string(dev.get_info(RS2_CAMERA_INFO_FIRMWARE_VERSION)));
            LOG_I("Serial Number:" + string(dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)));
            LOG_I("Product Id:" + string(dev.get_info(RS2_CAMERA_INFO_PRODUCT_ID)));

            auto cStream = m_rsProfile.get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>();
            m_cIntrinsics = cStream.get_intrinsics();
            auto dStream = m_rsProfile.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();
            m_dIntrinsics = dStream.get_intrinsics();

            // Depth sensor config
            auto dSensor = m_rsProfile.get_device().first<rs2::depth_sensor>();
            m_dScale = dSensor.get_depth_scale();

            auto range = dSensor.get_option_range(RS2_OPTION_VISUAL_PRESET);
            for (auto i = range.min; i <= range.max; i += range.step)
            {
                string preset = std::string(dSensor.get_option_value_description(RS2_OPTION_VISUAL_PRESET, i));
                IF_CONT(preset != m_vPreset);
                dSensor.set_option(RS2_OPTION_VISUAL_PRESET, i);
                break;
            }

            setSensorOption(dSensor, RS2_OPTION_CONFIDENCE_THRESHOLD, m_rsCtrl.m_fConfidenceThreshold);
            //            setSensorOption(dSensor, RS2_OPTION_DIGITAL_GAIN, m_rsCtrl.m_fDigitalGain);
            setSensorOption(dSensor, RS2_OPTION_PRE_PROCESSING_SHARPENING, m_rsCtrl.m_fPostProcessingSharpening);
            setSensorOption(dSensor, RS2_OPTION_FILTER_MAGNITUDE, m_rsCtrl.m_fFilterMagnitude);
            setSensorOption(dSensor, RS2_OPTION_HOLES_FILL, m_rsCtrl.m_fHolesFill);
            setSensorOption(dSensor, RS2_OPTION_EMITTER_ENABLED, m_rsCtrl.m_fEmitter);
            setSensorOption(dSensor, RS2_OPTION_LASER_POWER, m_rsCtrl.m_fLaserPower);

            // RGB sensor config
            auto cSensor = m_rsProfile.get_device().first<rs2::color_sensor>();
            setSensorOption(cSensor, RS2_OPTION_BRIGHTNESS, m_rsCtrl.m_fBrightness);
            setSensorOption(cSensor, RS2_OPTION_CONTRAST, m_rsCtrl.m_fContrast);
            setSensorOption(cSensor, RS2_OPTION_GAIN, m_rsCtrl.m_fGain);
            setSensorOption(cSensor, RS2_OPTION_BRIGHTNESS, m_rsCtrl.m_fExposure);
            setSensorOption(cSensor, RS2_OPTION_HUE, m_rsCtrl.m_fHue);
            setSensorOption(cSensor, RS2_OPTION_SATURATION, m_rsCtrl.m_fSaturation);
            setSensorOption(cSensor, RS2_OPTION_SHARPNESS, m_rsCtrl.m_fSharpness);
            setSensorOption(cSensor, RS2_OPTION_WHITE_BALANCE, m_rsCtrl.m_fWhiteBalance);

            // Confirm the frame is received
            rs2::frameset rsFrameset = m_rsPipe.wait_for_frames();

            if (m_bRGB)
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

                m_vSizeRGB.x = m_rsColor.as<rs2::video_frame>().get_width();
                m_vSizeRGB.y = m_rsColor.as<rs2::video_frame>().get_height();
            }
            else
            {
                m_rsDepth = rsFrameset.get_depth_frame();
            }

            if (m_rsCtrl.m_fFilterMagnitude < m_rsCtrl.m_fDefault)
                m_rsDepth = m_rsfDec.process(m_rsDepth);
            if (m_rsCtrl.m_fHolesFill < m_rsCtrl.m_fDefault)
                m_rsDepth = m_rsfSpat.process(m_rsDepth);

            m_vSizeD.x = m_rsDepth.as<rs2::video_frame>().get_width();
            m_vSizeD.y = m_rsDepth.as<rs2::video_frame>().get_height();
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

        // m_spImg = std::make_shared<geometry::Image>();
        // m_spImg->Prepare(m_vSizeRGB.x, m_vSizeRGB.y, 3, 1);

        m_bOpen = true;
        return true;
    }

    bool _RealSense::setSensorOption(const rs2::sensor &sensor, rs2_option option_type, float v)
    {
        if (!sensor.supports(option_type))
        {
            LOG_E("This option is not supported by this sensor");
            return false;
        }

        rs2::option_range range = sensor.get_option_range(option_type);
        if (v >= m_rsCtrl.m_fDefault)
        {
            v = range.def;
        }
        else
        {
            vFloat2 vRange(range.min, range.max);
            v = vRange.constrain(v);
        }

        try
        {
            sensor.set_option(option_type, v);
        }
        catch (const rs2::error &e)
        {
            LOG_E("Failed to set option: " + i2str(option_type) + ": " + string(e.what()));
            return false;
        }

        return true;
    }

    bool _RealSense::setCsensorOption(rs2_option option_type, float v)
    {
        auto cSensor = m_rsProfile.get_device().first<rs2::color_sensor>();
        setSensorOption(cSensor, option_type, v);
    }

    bool _RealSense::setDsensorOption(rs2_option option_type, float v)
    {
        auto dSensor = m_rsProfile.get_device().first<rs2::depth_sensor>();
        m_dScale = dSensor.get_depth_scale();

        setSensorOption(dSensor, option_type, v);
    }

    bool _RealSense::getSensorOption(const rs2::sensor &sensor, rs2_option option_type, rs2::option_range *pR)
    {
        NULL_F(pR);

        if (!sensor.supports(option_type))
        {
            LOG_E("This option is not supported by this sensor");
            return false;
        }

        *pR = sensor.get_option_range(option_type);

        return true;
    }

    bool _RealSense::getCsensorOption(rs2_option option_type, rs2::option_range *pR)
    {
        auto cSensor = m_rsProfile.get_device().first<rs2::color_sensor>();
        IF_F(!getSensorOption(cSensor, option_type, pR));

        return true;
    }

    bool _RealSense::getDsensorOption(rs2_option option_type, rs2::option_range *pR)
    {
        auto dSensor = m_rsProfile.get_device().first<rs2::depth_sensor>();
        IF_F(!getSensorOption(dSensor, option_type, pR));

        return true;
    }

    void _RealSense::sensorReset(void)
    {
        //    m_rsConfig.resolve(m_rsPipe).get_device().hardware_reset();
        rs2::device dev = m_rsProfile.get_device();
        dev.hardware_reset();
    }

    void _RealSense::close(void)
    {
        this->_RGBDbase::close();
        m_rsPipe.stop();
    }

    bool _RealSense::start(void)
    {
        NULL_F(m_pT);
        NULL__(m_pTpp);
        IF_F(!m_pT->start(getUpdate, this));
        return m_pTpp->start(getTPP, this);
    }

    bool _RealSense::check(void)
    {
        NULL_F(m_pT);
        NULL__(m_pTpp);

        return _RGBDbase::check();
    }

    void _RealSense::update(void)
    {
        while (m_pT->bAlive())
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

            m_pT->autoFPS();

            if (updateRS())
            {
                m_pTpp->wakeUp();
            }
            else
            {
                sensorReset();
                m_pT->sleepT(SEC_2_USEC);
                m_bOpen = false;
            }
        }
    }

    bool _RealSense::updateRS(void)
    {
        IF_F(!check());

        try
        {
            rs2::frameset rsFrameset = m_rsPipe.wait_for_frames();

            if (m_bRGB)
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

#ifdef USE_OPENCV
                m_fRGB.copy(Mat(Size(m_vSizeRGB.x, m_vSizeRGB.y), CV_8UC3, (void *)m_rsColor.get_data(), Mat::AUTO_STEP));
#endif
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

        return true;
    }

    void _RealSense::updateTPP(void)
    {
        while (m_pTpp->bAlive())
        {
            m_pTpp->sleepT(0);

            if (m_rsCtrl.m_fFilterMagnitude < m_rsCtrl.m_fDefault)
                m_rsDepth = m_rsfDec.process(m_rsDepth);
            if (m_rsCtrl.m_fHolesFill < m_rsCtrl.m_fDefault)
                m_rsDepth = m_rsfSpat.process(m_rsDepth);

#ifdef USE_OPENCV
            Mat mZ = Mat(Size(m_vSizeD.x, m_vSizeD.y), CV_16UC1, (void *)m_rsDepth.get_data(), Mat::AUTO_STEP);
            Mat mD, mDs;
            mZ.convertTo(mD, CV_32FC1);
            mDs = mD * m_dScale;
            m_fDepth.copy(mDs + m_dOfs);

            // if (m_bDepthShow)
            // {
            //     IF_(m_fDepth.bEmpty());
            //     rs2::colorizer rsColorMap;
            //     rs2::frame dColor = rsColorMap.process(m_rsDepth);
            //     Mat mDColor(Size(m_vDsize.x, m_vDsize.y), CV_8UC3, (void *)dColor.get_data(),
            //                 Mat::AUTO_STEP);
            //     m_fDepthShow.copy(mDColor);
            // }
#endif

            updatePointCloud();
        }
    }

#ifdef WITH_3D
    int _RealSense::getPointCloud(_PCframe *pPCframe, int nPmax)
    {
        NULL__(pPCframe, -1);
        PointCloud *pPC = pPCframe->getNextBuffer();

        m_rsPC.map_to(m_rsColor);
        m_rsPoints = m_rsPC.calculate(m_rsDepth);

        // memcpy(m_spImg->data_.data(),
        // 	   m_rsColor.get_data(),
        // 	   m_vSizeRGB.area() * 3);

        // auto rspVertex = m_rsPoints.get_vertices();
        // auto rspTexCoord = m_rsPoints.get_texture_coordinates();
        // int nP = m_rsPoints.size();

        // PointCloud *pPC = m_sPC.next();

        // const static float c_b = 1.0 / 255.0;

        // for (int i = 0; i < nP; i++)
        // {
        // 	rs2::vertex vr = rspVertex[i];
        // 	IF_CONT(vr.z < m_vRz.x);
        // 	IF_CONT(vr.z > m_vRz.y);

        // 	Eigen::Vector3d ve(vr.x, vr.y, vr.z);
        // 	pPC->points_.push_back(ve);

        // 	rs2::texture_coordinate tc = rspTexCoord[i];
        // 	int tx = constrain<int>(tc.u * m_vSizeRGB.x, 0, m_vSizeRGB.x - 1);
        // 	int ty = constrain<int>(tc.v * m_vSizeRGB.y, 0, m_vSizeRGB.y - 1);
        // 	Eigen::Vector3d te((double)*m_spImg->PointerAt<uint8_t>(tx, ty, 2),
        // 					   (double)*m_spImg->PointerAt<uint8_t>(tx, ty, 1),
        // 					   (double)*m_spImg->PointerAt<uint8_t>(tx, ty, 0));
        // 	te *= c_b;
        // 	pPC->colors_.push_back(te);
        // }

        return 0;
    }
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

}
