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
        m_type = vision_realsense;
        m_pTPP = NULL;

        m_rsSN = "";
        m_vPreset = "High Density";
        m_bRsRGB = true;
        m_rsFPS = 30;
        m_rsDFPS = 30;
        m_bAlign = false;
        m_rspAlign = NULL;

        m_fDefault = 10e6; //set to default value

        m_fConfidenceThreshold = m_fDefault;
        m_fDigitalGain = m_fDefault;
        m_fPostProcessingSharpening = m_fDefault;
        m_fFilterManitude = m_fDefault;
        m_fHolesFill = m_fDefault;
        m_fEmitter = m_fDefault;
        m_fLaserPower = m_fDefault;

        m_fBrightness = m_fDefault;
        m_fContrast = m_fDefault;
        m_fGain = m_fDefault;
        m_fExposure = m_fDefault;
        m_fHue = m_fDefault;
        m_fSaturation = m_fDefault;
        m_fSharpness = m_fDefault;
        m_fWhiteBalance = m_fDefault;
    }

    _RealSense::~_RealSense()
    {
        DEL(m_pTPP);
        DEL(m_rspAlign);
    }

    bool _RealSense::init(void *pKiss)
    {
        IF_F(!_DepthVisionBase::init(pKiss));
        Kiss *pK = (Kiss *)pKiss;

        pK->v("rsSN", &m_rsSN);
        pK->v("rsFPS", &m_rsFPS);
        pK->v("rsDFPS", &m_rsDFPS);
        pK->v("vPreset", &m_vPreset);
        pK->v("bRsRGB", &m_bRsRGB);
        pK->v("bAlign", &m_bAlign);

        pK->v("fConfidenceThreshold", &m_fConfidenceThreshold);
        pK->v("fDigitalGain", &m_fDigitalGain);
        pK->v("fPostProcessingSharpening", &m_fPostProcessingSharpening);
        pK->v("fFilterMagnitude", &m_fFilterManitude);
        pK->v("fHolesFill", &m_fHolesFill);
        pK->v("fEmitter", &m_fEmitter);
        pK->v("fLaserPower", &m_fLaserPower);

        pK->v("fBrightness", &m_fBrightness);
        pK->v("fContrast", &m_fContrast);
        pK->v("fGain", &m_fGain);
        pK->v("fExposure", &m_fExposure);
        pK->v("fHue", &m_fHue);
        pK->v("fSaturation", &m_fSaturation);
        pK->v("fSharpness", &m_fSharpness);
        pK->v("fWhiteBalance", &m_fWhiteBalance);

        Kiss *pKt = pK->child("threadPP");
        IF_F(pKt->empty());

        m_pTPP = new _Thread();
        if (!m_pTPP->init(pKt))
        {
            DEL(m_pTPP);
            return false;
        }

        return true;
    }

    bool _RealSense::open(void)
    {
        IF_T(m_bOpen);

        try
        {
            if (!m_rsSN.empty())
                m_rsConfig.enable_device(m_rsSN);

            m_rsConfig.enable_stream(RS2_STREAM_DEPTH, m_vDsize.x, m_vDsize.y, RS2_FORMAT_Z16, m_rsDFPS);
            if (m_bRsRGB)
                m_rsConfig.enable_stream(RS2_STREAM_COLOR, m_vSize.x, m_vSize.y, RS2_FORMAT_BGR8, m_rsFPS);

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

            setSensorOption(dSensor, RS2_OPTION_CONFIDENCE_THRESHOLD, m_fConfidenceThreshold);
            setSensorOption(dSensor, RS2_OPTION_DIGITAL_GAIN, m_fDigitalGain);
            setSensorOption(dSensor, RS2_OPTION_PRE_PROCESSING_SHARPENING, m_fPostProcessingSharpening);
            setSensorOption(dSensor, RS2_OPTION_FILTER_MAGNITUDE, m_fFilterManitude);
            setSensorOption(dSensor, RS2_OPTION_HOLES_FILL, m_fHolesFill);
            setSensorOption(dSensor, RS2_OPTION_EMITTER_ENABLED, m_fEmitter);
            setSensorOption(dSensor, RS2_OPTION_LASER_POWER, m_fLaserPower);

            // RGB sensor config
            auto cSensor = m_rsProfile.get_device().first<rs2::color_sensor>();
            setSensorOption(cSensor, RS2_OPTION_BRIGHTNESS, m_fBrightness);
            setSensorOption(cSensor, RS2_OPTION_CONTRAST, m_fContrast);
            setSensorOption(cSensor, RS2_OPTION_GAIN, m_fGain);
            setSensorOption(cSensor, RS2_OPTION_BRIGHTNESS, m_fExposure);
            setSensorOption(cSensor, RS2_OPTION_HUE, m_fHue);
            setSensorOption(cSensor, RS2_OPTION_SATURATION, m_fSaturation);
            setSensorOption(cSensor, RS2_OPTION_SHARPNESS, m_fSharpness);
            setSensorOption(cSensor, RS2_OPTION_WHITE_BALANCE, m_fWhiteBalance);

            // Confirm the frame is received
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

                m_vSize.x = m_rsColor.as<rs2::video_frame>().get_width();
                m_vSize.y = m_rsColor.as<rs2::video_frame>().get_height();
            }
            else
            {
                m_rsDepth = rsFrameset.get_depth_frame();
            }

            if (m_fFilterManitude < m_fDefault)
                m_rsDepth = m_rsfDec.process(m_rsDepth);
            if (m_fHolesFill < m_fDefault)
                m_rsDepth = m_rsfSpat.process(m_rsDepth);

            m_vDsize.x = m_rsDepth.as<rs2::video_frame>().get_width();
            m_vDsize.y = m_rsDepth.as<rs2::video_frame>().get_height();
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

        m_bOpen = true;
        return true;
    }

    bool _RealSense::setSensorOption(const rs2::sensor &sensor, rs2_option option_type, float v)
    {
        if (!sensor.supports(option_type))
        {
            LOG_E("This option is not supported by this sensor");
            return true;
        }

        rs2::option_range range = sensor.get_option_range(option_type);
        if (v >= m_fDefault)
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

    void _RealSense::sensorReset(void)
    {
        //    m_rsConfig.resolve(m_rsPipe).get_device().hardware_reset();
        rs2::device dev = m_rsProfile.get_device();
        dev.hardware_reset();
    }

    void _RealSense::close(void)
    {
        this->_DepthVisionBase::close();
        m_rsPipe.stop();
    }

    bool _RealSense::start(void)
    {
        NULL_F(m_pT);
        NULL_F(m_pTPP);
        IF_F(!m_pT->start(getUpdate, this));
        return m_pTPP->start(getTPP, this);
    }

    int _RealSense::check(void)
    {
        NULL__(m_pT, -1);
        NULL__(m_pTPP, -1);

        return _DepthVisionBase::check();
    }

    void _RealSense::update(void)
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
                m_pTPP->wakeUp();
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

    bool _RealSense::updateRS(void)
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

                m_fBGR.copy(Mat(Size(m_vSize.x, m_vSize.y), CV_8UC3, (void *)m_rsColor.get_data(), Mat::AUTO_STEP));
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
        while (m_pTPP->bRun())
        {
            m_pTPP->sleepT(0);

            if (m_fFilterManitude < m_fDefault)
                m_rsDepth = m_rsfDec.process(m_rsDepth);
            if (m_fHolesFill < m_fDefault)
                m_rsDepth = m_rsfSpat.process(m_rsDepth);

            Mat mZ = Mat(Size(m_vDsize.x, m_vDsize.y), CV_16UC1, (void *)m_rsDepth.get_data(), Mat::AUTO_STEP);
            Mat mD, mDs;
            mZ.convertTo(mD, CV_32FC1);
            mDs = mD * m_dScale;
            m_fDepth.copy(mDs + m_dOfs);

            if (m_bDepthShow)
            {
                IF_(m_fDepth.bEmpty());
                rs2::colorizer rsColorMap;
                rs2::frame dColor = rsColorMap.process(m_rsDepth);
                Mat mDColor(Size(m_vDsize.x, m_vDsize.y), CV_8UC3, (void *)dColor.get_data(),
                            Mat::AUTO_STEP);
                m_fDepthShow.copy(mDColor);
            }
        }
    }

}
