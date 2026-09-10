#pragma once
#include <RE/T/ThumbstickEvent.h>
#include <RE/P/PlayerInputHandler.h>
#include <RE/M/MenuEventHandler.h>
#include <pch.hpp>
#include "runtime_state.hpp"

class EventManager final : 
	public RE::BSTEventSink<RE::InputEvent*>,
	public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
	static EventManager& get()
	{
		static EventManager self;
		return self;
	}
	
	void update(RE::Main*, std::uintptr_t real_delta_address);
	void on_skse_message(std::uint32_t message_type);
	RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const*, RE::BSTEventSource<RE::InputEvent*>*) override;
	RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent*, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

	void mark_input_loaded();

private:
	[[nodiscard]] bool race_menu_is_open() const;
	[[nodiscard]] static RE::PlayerCharacter* get_ready_player();
	void reset_transient_state(std::string_view reason);

	RotationRuntimeState runtime_state;
	RE::NiPoint3 angle { 0.f, 0.f, 0.f };
	bool reported_missing_3d { false };
	bool reported_invalid_delta { false };
};

inline EventManager& EVENT_MANAGER { EventManager::get() };
