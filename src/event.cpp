#include "config.hpp"
#include "event.hpp"

void EventManager::update(float delta)
{
	if (const auto player = RE::PlayerCharacter::GetSingleton(); player && allow_rotate)
	{
		const float delta_x = mouse_delta_x != 0.f ? mouse_delta_x : gamepad_delta_x;
		if (delta_x != 0.f)
		{
			if (auto root = player->Get3D(false))
			{
				const float dir = delta_x > 0.f ? -1.f : 1.f;
				const float speed_t = std::min(std::abs(delta_x) / 360.f, 1.f);
				angle.z += dir * delta * std::lerp(config::min_rotate_speed, config::max_rotate_speed, speed_t);

				root->local.rotate.SetEulerAnglesXYZ(angle);
				// No UpdateWorldData call — let the engine update naturally next frame
			}
		}
	}
	mouse_delta_x = 0.f;
	gamepad_delta_x = 0.f;
}

RE::BSEventNotifyControl EventManager::ProcessEvent(RE::InputEvent* const* event, [[maybe_unused]] RE::BSTEventSource<RE::InputEvent*>* event_source)
{
	if (!event)
		return RE::BSEventNotifyControl::kContinue;

	if (const auto ui = RE::UI::GetSingleton(); ui && ui->IsMenuOpen(RE::RaceSexMenu::MENU_NAME))
	{
		for (auto input_event = *event; input_event; input_event = input_event->next)
		{
			switch (input_event->GetEventType())
			{
				case RE::INPUT_EVENT_TYPE::kButton:
				{
					auto button_event = input_event->AsButtonEvent();
					// Gamepad rotation is gated by the thumbstick, not a button
					if (!button_event || button_event->GetDevice() == RE::INPUT_DEVICE::kGamepad)
						continue;

					if (!button_event->IsHeld())
					{
						allow_rotate = false;
						continue;
					}

					switch (button_event->GetDevice())
					{
						case RE::INPUT_DEVICE::kKeyboard:
							if (const auto key = button_event->GetIDCode(); key == config::key_code)
							{
								allow_rotate = true;
								continue;
							}
							break;
						case RE::INPUT_DEVICE::kMouse:
							if (config::key_code >= 0x100 && button_event->GetIDCode() == config::key_code - 0x100)
							{
								allow_rotate = true;
								continue;
							}
							break;
					}
					continue;
				}
				case RE::INPUT_EVENT_TYPE::kMouseMove:
				{
					auto mouse_event = reinterpret_cast<RE::MouseMoveEvent*>(input_event->AsIDEvent());
					mouse_delta_x = static_cast<float>(mouse_event->mouseInputX);
					break;
				}
				case RE::INPUT_EVENT_TYPE::kThumbstick:
				{
					auto thumbstick_event = reinterpret_cast<RE::ThumbstickEvent*>(input_event->AsIDEvent());
					if (thumbstick_event->IsRight())
					{
						// Full deflection (|xValue| == 1) maps to max_rotate_speed, matching a 360-unit mouse delta
						gamepad_delta_x = thumbstick_event->xValue * 360.f;
						allow_rotate = gamepad_delta_x != 0.f;
					}
					break;
				}
			}
		}
	}
	return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventManager::ProcessEvent(const RE::MenuOpenCloseEvent* event, [[maybe_unused]] RE::BSTEventSource<RE::MenuOpenCloseEvent>* event_source)
{
	if (event && event->menuName == RE::RaceSexMenu::MENU_NAME)
	{
		if (event->opening)
		{
			if (auto player = RE::PlayerCharacter::GetSingleton())
				if (auto root = player->Get3D(false))
					root->local.rotate.ToEulerAnglesXYZ(angle);
		}
		else
		{
			allow_rotate = false;
			mouse_delta_x = 0.f;
			gamepad_delta_x = 0.f;
		}
	}
	return RE::BSEventNotifyControl::kContinue;
}