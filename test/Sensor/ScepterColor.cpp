#include "../../src/Vision/RGBD/_Scepter.h"
#include "../../src/Protocol/_JSONbase.h"
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <thread>

namespace kai
{
	struct ScepterColorTestAccess
	{
		static void attach(_Scepter &driver, PCLframe &cloud)
		{
			driver.m_scDevHandle = reinterpret_cast<ScDeviceHandle>(1);
			driver.m_bOpened = true;
			driver.m_pPCLout = &cloud;
			driver.m_bRGB = false;
			driver.m_bDepth = true;
			driver.m_bPCL = driver.m_bPCLrgb = driver.m_btRGB = true;
		}
		static bool capture(_Scepter &driver) { return driver.updateScRGBD(); }
		static bool openForWork(_Scepter &driver)
		{
			auto lock = driver.lockScForWork();
			return driver.open();
		}
		static bool commandWaiting(_Scepter &driver)
		{
			if (!driver.m_scAdmission.try_lock()) return true;
			driver.m_scAdmission.unlock();
			return false;
		}
		static void publish(_Scepter &driver) { driver.updatePCL(); }
		static bool pending(_Scepter &driver)
		{
			std::lock_guard<std::mutex> lock(driver.m_mutexPCL);
			return driver.m_bPCLframe;
		}
		static uint64_t depthStamp(_Scepter &driver)
		{
			std::lock_guard<std::mutex> lock(driver.m_mutexPCL);
			return driver.m_pclFrame.m_tStamp;
		}
		static void maxAge(_Scepter &driver, int milliseconds) { driver.m_pclColorMaxAgeMs = milliseconds; }
		static void expireColor(_Scepter &driver)
		{
			std::lock_guard<std::mutex> lock(driver.m_mutexPCL);
			driver.m_tColorPCL = getTns() - uint64_t(driver.m_pclColorMaxAgeMs + 1) * NSEC_MSEC;
		}
		static void disableColor(_Scepter &driver, bool alignment)
		{
			if (alignment) driver.m_btRGB = false;
			else driver.m_bPCLrgb = false;
		}
		static bool config(_Scepter &driver, const json &patch, bool device, json &errors)
		{
			return driver.applyConfig(patch, device, errors);
		}
		static json values(_Scepter &driver) { return driver.configValues(); }
	};
}

namespace
{
	using namespace kai;
	using Access = ScepterColorTestAccess;

	// Each SDK capture reuses the same buffers, including captures with no new
	// depth. Tests deliberately overwrite them before the PCL worker runs.
	struct FakeSDK
	{
		ScStatus status = SC_OK;
		ScFrameReady ready{};
		ScFrame depth{}, color{};
		std::vector<uint16_t> depthPixels;
		std::vector<uint8_t> colorPixels;
		unsigned conversions = 0;

		void capture(bool depthReady, bool colorReady, uint16_t millimeters = 1000,
			uint64_t depthTime = 1000, uint64_t colorTime = 1000,
			ScPixelFormat format = SC_PIXEL_FORMAT_BGR_888, uint16_t colorWidth = 2)
		{
			status = SC_OK;
			ready = {};
			ready.depth = depthReady;
			ready.transformedColor = colorReady;
			depthPixels.assign(2, millimeters);
			colorPixels.resize(size_t(colorWidth) * 3);
			for (size_t i = 0; i < colorWidth; ++i)
			{
				colorPixels[i * 3] = 11;
				colorPixels[i * 3 + 1] = 73;
				colorPixels[i * 3 + 2] = 219;
			}
			depth = {1, SC_DEPTH_FRAME, SC_PIXEL_FORMAT_DEPTH_MM16,
				reinterpret_cast<uint8_t *>(depthPixels.data()), uint32_t(depthPixels.size() * 2), 2, 1, depthTime};
			color = {1, SC_TRANSFORM_COLOR_IMG_TO_DEPTH_SENSOR_FRAME, format,
				colorPixels.data(), uint32_t(colorPixels.size()), colorWidth, 1, colorTime};
		}
	} sdk;

	struct BlockedCapture
	{
		std::mutex mutex;
		std::condition_variable condition;
		bool enabled = false, entered = false, released = false;
		bool blockFollowing = false, nextEntered = false, nextReleased = false;
	} blockedCapture;

	class ReplyTransport : public _JSONbase
	{
	public:
		bool sendJson(const json &value) override { reply = value; return true; }
		json reply;
	};

	struct Fixture
	{
		PCLframe cloud;
		_Scepter driver;
		Fixture() { sdk = {}; Access::attach(driver, cloud); }
		std::vector<GEOMETRY_POINT> publish()
		{
			const uint64_t captureStamp = Access::depthStamp(driver);
			Access::publish(driver);
			std::vector<GEOMETRY_POINT> points;
			assert(captureStamp != 0 && cloud.get(points) == captureStamp);
			for (const auto &point : points) assert(point.m_tStamp == captureStamp);
			return points;
		}
	};

	void expectCloud(const std::vector<GEOMETRY_POINT> &points, float depth, const Vector3f &color)
	{
		assert(points.size() == 2);
		for (const auto &point : points)
		{
			assert(std::abs(point.m_vP.z() - depth) < 1e-6f);
			assert(point.m_vC.isApprox(color));
			assert(point.m_tStamp != 0);
		}
	}
	const Vector3f bgrColor(219.f / 255, 73.f / 255, 11.f / 255);
	const Vector3f white(1, 1, 1);

	void independentStreamsAndOwnedBuffers()
	{
		Fixture f;
		sdk.capture(false, true);
		assert(Access::capture(f.driver));
		assert(!Access::pending(f.driver));
		std::fill(sdk.colorPixels.begin(), sdk.colorPixels.end(), 255);

		// A separate depth notification uses the retained owned aligned color.
		sdk.capture(true, false, 1200, 1100);
		assert(Access::capture(f.driver));
		std::fill(sdk.depthPixels.begin(), sdk.depthPixels.end(), 9000);
		expectCloud(f.publish(), 1.2f, bgrColor);
		const unsigned converted = sdk.conversions;
		const uint64_t published = f.cloud.getTstamp();
		Access::publish(f.driver);
		assert(sdk.conversions == converted && f.cloud.getTstamp() == published);

		// Color arriving after depth must not drop the pending depth frame.
		sdk.capture(true, false, 1700, 1200);
		assert(Access::capture(f.driver));
		sdk.capture(false, true, 8000, 0, 1250);
		assert(Access::capture(f.driver));
		expectCloud(f.publish(), 1.7f, bgrColor);
	}

	void timeoutAndLatestDepth()
	{
		Fixture f;
		sdk.capture(true, true, 1400);
		assert(Access::capture(f.driver));
		sdk.status = SC_GET_FRAME_READY_TIME_OUT;
		std::fill(sdk.depthPixels.begin(), sdk.depthPixels.end(), 8000);
		std::fill(sdk.colorPixels.begin(), sdk.colorPixels.end(), 255);
		assert(!Access::capture(f.driver));
		assert(Access::pending(f.driver));
		expectCloud(f.publish(), 1.4f, bgrColor);

		// The bounded pending slot retains the newest valid depth, not a backlog.
		sdk.capture(true, false, 1500, 1100);
		assert(Access::capture(f.driver));
		sdk.capture(true, false, 1900, 1200);
		assert(Access::capture(f.driver));
		expectCloud(f.publish(), 1.9f, bgrColor);

		// A malformed SDK frame cannot replace the last valid pending input.
		sdk.capture(true, true, 1800, 1300, 1300);
		assert(Access::capture(f.driver));
		sdk.capture(true, true, 5000, 1400, 1400);
		sdk.depth.dataLen = sdk.color.dataLen = 0;
		assert(Access::capture(f.driver));
		expectCloud(f.publish(), 1.8f, bgrColor);
	}

	void colorValidityAndFallback()
	{
		for (int scenario = 0; scenario < 6; ++scenario)
		{
			Fixture f;
			if (scenario == 5) Access::disableColor(f.driver, true);
			sdk.capture(true, scenario != 0, 1000,
				scenario == 2 ? 2501 : 1000,
				scenario == 3 ? 2501 : 1000,
				SC_PIXEL_FORMAT_BGR_888, scenario == 4 ? 1 : 2);
			assert(Access::capture(f.driver));
			if (scenario == 1) Access::expireColor(f.driver);
			// Missing, old, future-skewed, mismatched, or disabled color must
			// never suppress current depth points used by obstacle avoidance.
			expectCloud(f.publish(), 1.f, white);
		}
		Fixture f;
		Access::disableColor(f.driver, false);
		sdk.capture(true, true);
		assert(Access::capture(f.driver));
		expectCloud(f.publish(), 1.f, white);
	}

	void publicationDuringNextCapture()
	{
		Fixture f;
		Access::maxAge(f.driver, 10000);
		sdk.capture(true, true, 1300);
		assert(Access::capture(f.driver));
		const uint64_t stamp = Access::depthStamp(f.driver);
		const unsigned conversions = sdk.conversions;

		// The next SDK wait owns the device mutex until explicitly released.
		// Publishing the previous frame must not depend on that mutex or make
		// another SDK call; otherwise a busy capture loop can starve the viewer.
		sdk.capture(false, false, 9000);
		{
			std::lock_guard<std::mutex> lock(blockedCapture.mutex);
			blockedCapture.enabled = true;
			blockedCapture.entered = blockedCapture.released = false;
		}
		auto capture = std::async(std::launch::async, [&] { return Access::capture(f.driver); });
		bool entered;
		{
			std::unique_lock<std::mutex> lock(blockedCapture.mutex);
			entered = blockedCapture.condition.wait_for(lock, std::chrono::seconds(2),
				[] { return blockedCapture.entered; });
		}
		auto publisher = std::async(std::launch::async, [&] { Access::publish(f.driver); });
		const bool publishedWhileBlocked = publisher.wait_for(std::chrono::seconds(1)) == std::future_status::ready;
		// Release and join before checking the result so regressions fail rather
		// than hanging while the future destructors wait for a blocked worker.
		{
			std::lock_guard<std::mutex> lock(blockedCapture.mutex);
			blockedCapture.released = true;
		}
		blockedCapture.condition.notify_all();
		const bool captured = capture.get();
		publisher.get();
		assert(entered && captured && publishedWhileBlocked);
		assert(sdk.conversions == conversions);
		std::vector<GEOMETRY_POINT> points;
		assert(f.cloud.get(points) == stamp);
		for (const auto &point : points) assert(point.m_tStamp == stamp);
		expectCloud(points, 1.3f, bgrColor);
	}

	void commandAdmissionBetweenCaptures()
	{
		Fixture f;
		f.driver.setName("scepter");
		ReplyTransport transport;
		sdk.capture(false, false);
		{
			std::lock_guard<std::mutex> lock(blockedCapture.mutex);
			blockedCapture.enabled = blockedCapture.blockFollowing = true;
			blockedCapture.entered = blockedCapture.released = false;
			blockedCapture.nextEntered = blockedCapture.nextReleased = false;
		}
		auto captures = std::async(std::launch::async, [&] {
			const bool first = Access::capture(f.driver);
			const bool opened = Access::openForWork(f.driver);
			return Access::capture(f.driver) && first && opened;
		});
		bool entered;
		{
			std::unique_lock<std::mutex> lock(blockedCapture.mutex);
			entered = blockedCapture.condition.wait_for(lock, std::chrono::seconds(2),
				[] { return blockedCapture.entered; });
		}
		auto command = std::async(std::launch::async, [&] {
			f.driver.console({{"module", "scepter"}, {"cmd", "getConfig"},
				{"requestId", "camera-admission-test"}}, &transport);
		});

		// A waiting command holds admission while the first capture owns the
		// SDK. Wait for that state before releasing capture, without relying on
		// scheduler timing or sleeping inside the production capture loop.
		bool queued = false;
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
		while (!(queued = Access::commandWaiting(f.driver)) && std::chrono::steady_clock::now() < deadline)
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		{
			std::lock_guard<std::mutex> lock(blockedCapture.mutex);
			blockedCapture.released = true;
		}
		blockedCapture.condition.notify_all();
		bool nextEntered;
		{
			std::unique_lock<std::mutex> lock(blockedCapture.mutex);
			nextEntered = blockedCapture.condition.wait_for(lock, std::chrono::seconds(2),
				[] { return blockedCapture.nextEntered; });
		}
		// The command must have acquired the device between captures, so its
		// reply can finish even while the next SDK wait remains blocked.
		const bool repliedWhileBlocked = command.wait_for(std::chrono::seconds(1)) == std::future_status::ready;
		{
			std::lock_guard<std::mutex> lock(blockedCapture.mutex);
			blockedCapture.nextReleased = true;
		}
		blockedCapture.condition.notify_all();
		const bool captured = captures.get();
		command.get();
		assert(entered && queued && nextEntered && captured && repliedWhileBlocked);
		assert(transport.reply.at("requestId") == "camera-admission-test");
		assert(transport.reply.at("module") == "scepter" && transport.reply.at("cmd") == "getConfig");
		assert(transport.reply.at("bSuccess") == true && transport.reply.at("deviceOpen") == true);
		assert(!transport.reply.at("schema").empty());
		assert(transport.reply.at("config").at("bPCLrgb") == true);
	}

	void pixelOrderAndZeroDeviceTimestamps()
	{
		for (auto format : {SC_PIXEL_FORMAT_BGR_888, SC_PIXEL_FORMAT_BGR_888_JPEG,
			SC_PIXEL_FORMAT_RGB_888, SC_PIXEL_FORMAT_RGB_888_JPEG})
		{
			Fixture f;
			sdk.capture(false, true, 1000, 0, 0, format);
			assert(Access::capture(f.driver));
			sdk.capture(true, false, 1000, 0, 0);
			assert(Access::capture(f.driver));
			const bool rgb = format == SC_PIXEL_FORMAT_RGB_888 || format == SC_PIXEL_FORMAT_RGB_888_JPEG;
			expectCloud(f.publish(), 1.f, rgb ? Vector3f(11.f / 255, 73.f / 255, 219.f / 255) : bgrColor);
		}
	}

	void sameCaptureOnly()
	{
		Fixture f;
		Access::maxAge(f.driver, 0);
		sdk.capture(true, true);
		assert(Access::capture(f.driver));
		expectCloud(f.publish(), 1.f, bgrColor);
		sdk.capture(true, false, 1200, 1100);
		assert(Access::capture(f.driver));
		expectCloud(f.publish(), 1.2f, white);
	}

	void resetRetainedFrames()
	{
		for (bool close : {false, true})
		{
			Fixture f;
			sdk.capture(true, true);
			assert(Access::capture(f.driver));
			if (close)
			{
				f.driver.close();
				Access::attach(f.driver, f.cloud);
			}
			else
			{
				json errors;
				assert(Access::config(f.driver, {{"pclColorMaxAgeMs", 500}}, true, errors));
			}
			assert(!Access::pending(f.driver));
			Access::publish(f.driver);
			assert(f.cloud.getTstamp() == 0);
			sdk.capture(true, false);
			assert(Access::capture(f.driver));
			expectCloud(f.publish(), 1.f, white);
		}
	}

	void configuration()
	{
		JsonCfg cfg;
		cfg.setJson({{"camera", {{"class", "_Scepter"}, {"thread", {{"FPS", 10}}},
			{"threadPP", {{"FPS", 10}}}, {"pclColorMaxAgeMs", 275}}}});
		json &settings = (*cfg.getJson())["camera"];
		_Scepter driver;
		driver.setName("camera");
		driver.setConfig(&cfg, &settings);
		assert(driver.loadConfig());
		assert(Access::values(driver).at("pclColorMaxAgeMs") == 275);
		assert(driver.saveConfig(false));
		assert(settings.at("pclColorMaxAgeMs") == 275);
		json errors;
		for (const json &invalid : {json(-1), json(10001), json(0.5), json("1000"), json(uint64_t(INT_MAX) + 1)})
		{
			assert(!Access::config(driver, {{"pclColorMaxAgeMs", invalid}}, false, errors));
			assert(errors.contains("pclColorMaxAgeMs"));
			assert(Access::values(driver).at("pclColorMaxAgeMs") == 275);
		}
		assert(Access::config(driver, {{"pclColorMaxAgeMs", 10000}}, false, errors));
		assert(Access::values(driver).at("pclColorMaxAgeMs") == 10000);
		assert(Access::config(driver, {{"pclColorMaxAgeMs", 0}}, false, errors));
		assert(driver.saveConfig(false));
		assert(settings.at("pclColorMaxAgeMs") == 0);
		assert(driver.loadConfig());
		assert(Access::values(driver).at("pclColorMaxAgeMs") == 0);
		settings["pclColorMaxAgeMs"] = -1;
		assert(!driver.loadConfig());
		_Scepter defaults;
		assert(Access::values(defaults).at("pclColorMaxAgeMs") == 1000);
	}
}

extern "C" ScStatus __wrap_scGetFrameReady(ScDeviceHandle device, uint16_t, ScFrameReady *ready)
{
	assert(device == reinterpret_cast<ScDeviceHandle>(1));
	{
		std::unique_lock<std::mutex> lock(blockedCapture.mutex);
		if (blockedCapture.enabled)
		{
			blockedCapture.entered = true;
			blockedCapture.condition.notify_all();
			blockedCapture.condition.wait(lock, [] { return blockedCapture.released; });
			blockedCapture.enabled = false;
		}
		else if (blockedCapture.blockFollowing)
		{
			blockedCapture.nextEntered = true;
			blockedCapture.condition.notify_all();
			blockedCapture.condition.wait(lock, [] { return blockedCapture.nextReleased; });
			blockedCapture.blockFollowing = false;
		}
	}
	*ready = sdk.ready;
	return sdk.status;
}

extern "C" ScStatus __wrap_scGetFrame(ScDeviceHandle, ScFrameType type, ScFrame *frame)
{
	if (type == SC_DEPTH_FRAME && sdk.ready.depth) *frame = sdk.depth;
	else if (type == SC_TRANSFORM_COLOR_IMG_TO_DEPTH_SENSOR_FRAME && sdk.ready.transformedColor) *frame = sdk.color;
	else { assert(false && "Unexpected SDK frame request"); return SC_INVALID_FRAME_TYPE; }
	return SC_OK;
}

extern "C" ScStatus __wrap_scConvertDepthFrameToPointCloudVector(ScDeviceHandle, const ScFrame *frame, ScVector3f *points)
{
	++sdk.conversions;
	assert(frame->pixelFormat == SC_PIXEL_FORMAT_DEPTH_MM16);
	assert(frame->dataLen >= size_t(frame->width) * frame->height * sizeof(uint16_t));
	const auto *depth = reinterpret_cast<const uint16_t *>(frame->pFrameData);
	for (size_t i = 0; i < size_t(frame->width) * frame->height; ++i)
		points[i] = {float(i), 0, float(depth[i])};
	return SC_OK;
}

extern "C" ScStatus __wrap_scStopStream(ScDeviceHandle) { return SC_OK; }
extern "C" ScStatus __wrap_scCloseDevice(ScDeviceHandle *device) { *device = nullptr; return SC_OK; }

int main()
{
	independentStreamsAndOwnedBuffers();
	timeoutAndLatestDepth();
	publicationDuringNextCapture();
	commandAdmissionBetweenCaptures();
	colorValidityAndFallback();
	pixelOrderAndZeroDeviceTimestamps();
	sameCaptureOnly();
	resetRetainedFrames();
	configuration();
	std::cout << "Scepter asynchronous color, nonblocking publication, command admission, owned frames, fallback, reset and configuration tests passed\n";
}
