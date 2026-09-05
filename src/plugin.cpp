#include "config.hpp"
#include "hooks.hpp"
#include "event.hpp"
#include "spdlog/logger.h"

// The CMake helper in older CommonLibSSE-NG releases emits an obsolete plugin
// declaration.  Define it directly so SKSE 2.3.x sees both Address Library use
// and the v5 database-format compatibility bit.
SKSEPluginVersion = []() noexcept
{
	auto version = SKSE::PluginVersionData{};
	version.PluginVersion({ 1, 2, 0, 0 });
	version.PluginName("PlayerRotationGPSupport"sv);
	version.AuthorName("kezc7"sv);
	version.UsesAddressLibrary();
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

	SKSE::Init(skse);
	config::load();
	hooks::install();
	
	if (const auto messageInterface = SKSE::GetMessagingInterface())
	{
		messageInterface->RegisterListener([](SKSE::MessagingInterface::Message* msg)
		{
			if (msg->type == SKSE::MessagingInterface::kInputLoaded)
			{
				if (const auto input = RE::BSInputDeviceManager::GetSingleton())
					input->AddEventSink<RE::InputEvent*>(&EVENT_MANAGER);
				if (const auto ui = RE::UI::GetSingleton())
					ui->AddEventSink<RE::MenuOpenCloseEvent>(&EVENT_MANAGER);
			}
		});
	}
	return true;
}
