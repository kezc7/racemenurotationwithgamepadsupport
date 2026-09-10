#include "config.hpp"
#include "hooks.hpp"
#include "event.hpp"
#include "spdlog/logger.h"

// The CMake helper in older CommonLibSSE-NG releases emits an obsolete plugin
// declaration.  Define it directly so SKSE 2.3.x sees Address Library use,
// post-1.6.629 structure compatibility, and the v5 database-format bit.
SKSEPluginVersion = []() noexcept
{
	auto version = SKSE::PluginVersionData{};
	version.PluginVersion({ 1, 3, 0, 0 });
	version.PluginName("PlayerRotationGPSupport"sv);
	version.AuthorName("kezc7"sv);
	version.UsesAddressLibrary();
	version.UsesUpdatedStructs();
	return version;
}();

extern "C" [[maybe_unused]] __declspec(dllexport) bool SKSEPlugin_Query(
	[[maybe_unused]] const SKSE::QueryInterface*, SKSE::PluginInfo* pluginInfo)
{
	pluginInfo->infoVersion = SKSE::PluginInfo::kVersion;
	pluginInfo->name = SKSEPlugin_Version.pluginName;
	pluginInfo->version = SKSEPlugin_Version.pluginVersion;
	return true;
}

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
#ifndef NDEBUG
	const auto level = spdlog::level::trace;
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	const auto level = spdlog::level::info;
	auto logPath = logger::log_directory();
	if (!logPath)
		return false;
	*logPath /= "PlayerRotation.log"sv;

	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath->string(), true);
#endif
	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%l] %v"s);
	logger::info("PlayerRotationGPSupport {} initializing"sv, SKSEPlugin_Version.pluginVersion);

	SKSE::Init(skse);
	config::load();
	if (!hooks::install())
		return false;
	
	if (const auto messageInterface = SKSE::GetMessagingInterface())
	{
		static bool input_sink_registered = false;
		static bool menu_sink_registered = false;
		static bool registration_logged = false;
		messageInterface->RegisterListener([](SKSE::MessagingInterface::Message* msg)
		{
			if (!msg)
				return;

			if (msg->type == SKSE::MessagingInterface::kInputLoaded ||
				msg->type == SKSE::MessagingInterface::kDataLoaded)
			{
				if (!input_sink_registered)
				{
					if (const auto input = RE::BSInputDeviceManager::GetSingleton())
					{
						input->AddEventSink<RE::InputEvent*>(&EVENT_MANAGER);
						input_sink_registered = true;
					}
				}
				if (!menu_sink_registered)
				{
					if (const auto ui = RE::UI::GetSingleton())
					{
						ui->AddEventSink<RE::MenuOpenCloseEvent>(&EVENT_MANAGER);
						menu_sink_registered = true;
					}
				}
				if (input_sink_registered && menu_sink_registered)
				{
					if (!registration_logged)
					{
						logger::info("Input and menu event sinks registered"sv);
						registration_logged = true;
					}
				}
				else if (!registration_logged)
				{
					logger::warn("InputLoaded received, but input/UI event sinks were unavailable"sv);
				}
				if (msg->type == SKSE::MessagingInterface::kInputLoaded)
					EVENT_MANAGER.mark_input_loaded();
				else
					EVENT_MANAGER.on_skse_message(msg->type);
			}
			else
			{
				EVENT_MANAGER.on_skse_message(msg->type);
			}
		});
	}
	else
	{
		logger::error("SKSE messaging interface unavailable; lifecycle gating cannot be enabled"sv);
		return false;
	}

	logger::info("PlayerRotationGPSupport initialization complete"sv);
	return true;
}
