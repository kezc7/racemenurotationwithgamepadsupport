#include "config.hpp"
#include "hooks.hpp"
#include "event.hpp"
#include "spdlog/logger.h"

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
#ifndef NDEBUG
	const auto level = spdlog::level::trace;
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	const auto level = spdlog::level::trace;
	auto logPath = logger::log_directory();
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