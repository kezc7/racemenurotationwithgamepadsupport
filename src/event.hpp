#pragma once
#include <RE/T/ThumbstickEvent.h>
#include <RE/P/PlayerInputHandler.h>
#include <RE/M/MenuEventHandler.h>
#include <pch.hpp>

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
	
	void update(float);
	RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const*, RE::BSTEventSource<RE::InputEvent*>*) override;
	RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent*, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

	bool allow_rotate;
	int32_t mouse_delta_x;
	int32_t gamepad_delta_x;
	RE::NiPoint3 angle;
};

static EventManager& EVENT_MANAGER { EventManager::get() };