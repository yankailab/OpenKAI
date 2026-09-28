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
        m_rsCtrl.clear();

        m_vSizeRGB = Vector2i(1280, 720);
        m_vSizeD = Vector2i(640, 480);
    }

    _RealSense::~_RealSense()
    {
        stop();
    }

    bool _RealSense::loadConfig(void)
    {
        IF_F(!_RGBDbase::loadConfig());
        const json &j = *m_pJ;

        jKv(j, "rsSN", m_rsSN);
        jKv(j, "rsFPS", m_rsFPS);
        jKv(j, "rsDFPS", m_rsDFPS);
        jKv(j, "bAlign", m_bAlign);
        jKv(j, "vPreset", m_vPreset);

        jKv(j, "fConfidenceThreshold", m_rsCtrl.m_fConfidenceThr);
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
        m_pTpp = createThread(jK(*m_pJ, "threadPP"), "threadPP");
        NULL_F(m_pTpp);

        return true;
    }

    bool _RealSense::saveConfig(bool bExport)
    {
        IF_F(!_RGBDbase::saveConfig(false));

        json &j = *m_pJ;
        j["rsSN"] = m_rsSN;
        j["rsFPS"] = m_rsFPS;
        j["rsDFPS"] = m_rsDFPS;
        j["bAlign"] = m_bAlign;
        j["vPreset"] = m_vPreset;
        j["fConfidenceThreshold"] = m_rsCtrl.m_fConfidenceThr;
        j["fDigitalGain"] = m_rsCtrl.m_fDigitalGain;
        j["fPostProcessingSharpening"] = m_rsCtrl.m_fPostProcessingSharpening;
        j["fFilterMagnitude"] = m_rsCtrl.m_fFilterMagnitude;
        j["fHolesFill"] = m_rsCtrl.m_fHolesFill;
        j["fEmitter"] = m_rsCtrl.m_fEmitter;
        j["fLaserPower"] = m_rsCtrl.m_fLaserPower;
        j["fBrightness"] = m_rsCtrl.m_fBrightness;
        j["fContrast"] = m_rsCtrl.m_fContrast;
        j["fGain"] = m_rsCtrl.m_fGain;
        j["fExposure"] = m_rsCtrl.m_fExposure;
        j["fHue"] = m_rsCtrl.m_fHue;
        j["fSaturation"] = m_rsCtrl.m_fSaturation;
        j["fSharpness"] = m_rsCtrl.m_fSharpness;
        j["fWhiteBalance"] = m_rsCtrl.m_fWhiteBalance;

        IF__(!bExport, true);
        return m_pJcfg->saveToFile();
    }

    bool _RealSense::open(void)
    {
        IF_F(m_bOpened);

        try
        {
            if (!m_rsSN.empty())
                m_rsConfig.enable_device(m_rsSN);

            m_rsConfig.enable_stream(RS2_STREAM_DEPTH, m_vSizeD.x(), m_vSizeD.y(), RS2_FORMAT_Z16, m_rsDFPS);
            if (m_bRGB)
                m_rsConfig.enable_stream(RS2_STREAM_COLOR, m_vSizeRGB.x(), m_vSizeRGB.y(), RS2_FORMAT_BGR8, m_rsFPS);

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

            setSensorOption(dSensor, RS2_OPTION_CONFIDENCE_THRESHOLD, m_rsCtrl.m_fConfidenceThr);
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
            rs2::frame rsColor;
            rs2::frame rsDepth;

            if (m_bRGB)
            {
                if (m_bAlign)
                {
                    rs2::align align(RS2_STREAM_COLOR);
                    rs2::frameset rsFramesetAlign = align.process(rsFrameset);
                    rsColor = rsFramesetAlign.get_color_frame();
                    rsDepth = rsFramesetAlign.get_depth_frame();
                }
                else
                {
                    rsColor = rsFrameset.get_color_frame();
                    rsDepth = rsFrameset.get_depth_frame();
                }

                m_vSizeRGB.x() = rsColor.as<rs2::video_frame>().get_width();
                m_vSizeRGB.y() = rsColor.as<rs2::video_frame>().get_height();
            }
            else
            {
                rsDepth = rsFrameset.get_depth_frame();
            }

            m_vSizeD.x() = rsDepth.as<rs2::video_frame>().get_width();
            m_vSizeD.y() = rsDepth.as<rs2::video_frame>().get_height();
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

        m_bOpened = true;
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
            Vector2f vRange(range.min, range.max);
            v = std::clamp(v, vRange.x(), vRange.y());
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
        if (m_bOpened)
        {
            try
            {
                m_rsPipe.stop();
            }
            catch (const rs2::error &e)
            {
                LOG_E(e.what());
            }
        }
        this->_RGBDbase::close();
    }

    bool _RealSense::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTpp);
        IF_F(!m_pT->startThread(getUpdate, this));
        return m_pTpp->startThread(getTPP, this);
    }

    void _RealSense::stop(void)
    {
        if (m_pT)
        {
            m_pT->join();
        }
        if (m_pTpp)
        {
            m_pTpp->join();
        }
        close();
    }

    bool _RealSense::check(void)
    {
        return _RGBDbase::check();
    }

    void _RealSense::update(void)
    {
        while (m_pT->bRun())
        {
            if (!m_bOpened)
            {
                if (!open())
                {
                    LOG_E("Cannot open RealSense");
                    sensorReset();
                    m_pT->sleepT(NSEC_SEC);
                    continue;
                }
            }

            m_pT->autoFPS();

            if (updateRS())
            {
                m_pTpp->run();
            }
            else
            {
                sensorReset();
                m_pT->sleepT(NSEC_SEC);
                m_bOpened = false;
            }
        }
    }

    bool _RealSense::updateRS(void)
    {
        IF_F(!check());

        try
        {
            rs2::frameset frames = m_rsPipe.wait_for_frames();
            m_rsFrames.enqueue(frames);
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
        rs2::align align(RS2_STREAM_COLOR);
        while (m_pTpp->bRun())
        {
            m_pTpp->autoFPS();
            rs2::frameset frames;
            if (!m_rsFrames.poll_for_frame(&frames))
            {
                continue;
            }

            try
            {
                if (m_bRGB && m_bAlign)
                {
                    frames = align.process(frames);
                }
                rs2::frame color = frames.get_color_frame();
                rs2::frame depth = frames.get_depth_frame();
                if (!depth)
                {
                    continue;
                }
                if (m_rsCtrl.m_fFilterMagnitude < m_rsCtrl.m_fDefault)
                {
                    depth = m_rsfDec.process(depth);
                }
                if (m_rsCtrl.m_fHolesFill < m_rsCtrl.m_fDefault)
                {
                    depth = m_rsfSpat.process(depth);
                }

                const auto videoDepth = depth.as<rs2::video_frame>();
                const uint64_t tStamp = static_cast<uint64_t>(depth.get_timestamp() * NSEC_MSEC);
                Mat mRaw(videoDepth.get_height(), videoDepth.get_width(), CV_16UC1,
                         const_cast<void *>(depth.get_data()), videoDepth.get_stride_in_bytes());
                Mat mDepth;
                mRaw.convertTo(mDepth, CV_32FC1, m_dScale, m_dOfs);
                if (m_pD)
                {
                    m_pD->set(mDepth, tStamp);
                }

                Mat mRGB;
                if (m_bRGB && color)
                {
                    const auto videoColor = color.as<rs2::video_frame>();
                    mRGB = Mat(videoColor.get_height(), videoColor.get_width(), CV_8UC3,
                               const_cast<void *>(color.get_data()), videoColor.get_stride_in_bytes());
                    if (m_pRGB)
                    {
                        m_pRGB->set(mRGB, static_cast<uint64_t>(color.get_timestamp() * NSEC_MSEC));
                    }
                    if (m_pRGBD)
                    {
                        m_pRGBD->set(mRGB, mDepth, tStamp);
                    }
                    if (m_bAlign && m_pRGBDtRGB)
                    {
                        m_pRGBDtRGB->set(mRGB, mDepth, tStamp);
                    }
                }
                updatePC(depth, color, mRGB, tStamp);
            }
            catch (const rs2::error &e)
            {
                LOG_E(e.what());
            }
        }
    }

    void _RealSense::updatePC(const rs2::frame &depth, const rs2::frame &color, const Mat &mRGB, uint64_t tStamp)
    {
        if (!m_pPCL || (!m_bPCL && !m_bPCLrgb))
        {
            return;
        }

        if (m_bPCLrgb && color)
        {
            m_rsPC.map_to(color);
        }
        const rs2::points points = m_rsPC.calculate(depth);
        const auto *vertices = points.get_vertices();
        const auto *texCoords = points.get_texture_coordinates();
        vector<GEOMETRY_POINT> vPCL;
        vPCL.reserve(points.size());
        for (size_t i = 0; i < points.size(); ++i)
        {
            const auto &p = vertices[i];
            if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z) || p.z <= 0)
            {
                continue;
            }

            Vector3f vC(1, 1, 1);
            if (m_bPCLrgb && !mRGB.empty())
            {
                const auto &uv = texCoords[i];
                if (std::isfinite(uv.u) && std::isfinite(uv.v))
                {
                    const int x = constrain<int>(uv.u * mRGB.cols, 0, mRGB.cols - 1);
                    const int y = constrain<int>(uv.v * mRGB.rows, 0, mRGB.rows - 1);
                    const Vec3b c = mRGB.at<Vec3b>(y, x);
                    vC = Vector3f(c[2], c[1], c[0]) / 255.0f;
                }
            }
            vPCL.push_back({Vector3f(p.x, p.y, p.z), vC, tStamp});
        }
        m_pPCL->set(std::move(vPCL), tStamp);
    }

}
