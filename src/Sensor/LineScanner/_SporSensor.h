/*
 * Spor Ethernet laser line scanner. Coordinates published to PCLframe are metres.
 */
#ifndef OpenKAI_src_Sensor_LineScanner__SporSensor_H_
#define OpenKAI_src_Sensor_LineScanner__SporSensor_H_

#include "../../Base/_ModuleBase.h"
#include "../../DataObject/PCLframe.h"
#include <spor/settings.hpp>
#include <spor/stream.hpp>
#include <memory>
#include <mutex>

namespace kai
{
	class _SporSensor : public _ModuleBase
	{
	public:
		_SporSensor();
		~_SporSensor() override;

		bool loadConfig(void) override;
		bool saveConfig(bool bExport) override;
		bool link(InstanceMgr *pM) override;
		bool start(void) override;
		bool check(void) override;
		using _ModuleBase::console;
		void console(void *pConsole) override;
		void console(const json &j, void *pJSONbase) override;
		bool open(void);
		void close(void);

	protected:
		json configValues(void) const;
		json controlSchema(void) const;
		bool applyConfig(const json &patch, bool device, json &errors);
		void publishFrame(const spor::ProfileFrame &frame);

		PCLframe *m_pPCLout = nullptr;
		json m_config;

	private:
		void update(void);
		static void *getUpdate(void *This)
		{
			static_cast<_SporSensor *>(This)->update();
			return nullptr;
		}
		void applyDeviceSettings(void);
		void refreshDevice(void);
		void connectStream(void);
		json status(void) const;

		// Control/UI operations are serialized. Only update() receives profiles,
		// using a separate SDK channel. Settings changes stop acquisition first.
		mutable std::recursive_mutex m_mutex;
		std::unique_ptr<spor::UiClient> m_ui;
		std::unique_ptr<spor::SettingsClient> m_settings;
		std::unique_ptr<spor::ControlClient> m_control;
		// A pending receive retains its old stream until it returns. Replacing a
		// subscriber never calls disconnect concurrently with receive().
		std::shared_ptr<spor::ProfileStream> m_stream;
		spor::StreamOptions m_streamOptions;
		bool m_opened = false;
		bool m_acquiring = false;
		bool m_wantAcquisition = true;
		uint64_t m_nextConnect = 0;
		uint64_t m_frameCount = 0;
		uint64_t m_frameId = 0;
		uint64_t m_rawTimestamp = 0;
		uint64_t m_encoder = 0;
		size_t m_pointCount = 0;
		string m_lastError;
		json m_errors = json::object();
		json m_device = json::object();
	};
}
#endif
