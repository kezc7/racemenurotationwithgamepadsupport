#include "config.hpp"
#include "event.hpp"

void EventManager::update(float delta)
{
	if (const auto player = RE::PlayerCharacter::GetSingleton(); player && allow_rotate)
	{
		if (std::abs(mouse_delta_x) == 0 && std::abs(gamepad_delta_x) == 0)
			return;

		if (auto root = player->Get3D(false))
		{
			int dir = (mouse_delta_x > 0 || gamepad_delta_x > 0) ? -1 : 1;
            float delta_x = std::abs(mouse_delta_x) > 0.0f ? static_cast<float>(mouse_delta_x) : static_cast<float>(gamepad_delta_x);
			angle.z += dir * delta * std::lerp(config::min_rotate_speed, config::max_rotate_speed, std::abs(delta_x) / 360.f);

			root->local.rotate.SetEulerAnglesXYZ(angle);

			RE::NiUpdateData data;
			root->UpdateWorldData(&data);
		}
	}
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
					if (!(button_event && button_event->IsHeld()))
					{
						allow_rotate = false;
						continue;
					}

					switch (input_event->GetDevice())
					{
						case RE::INPUT_DEVICE::kKeyboard:
							if (const auto key = button_event->GetIDCode(); key == config::key_code)
							{
								allow_rotate = true;
								continue;
							}
							break;
						case RE::INPUT_DEVICE::kMouse:
							if (const auto mouseButton = button_event->GetIDCode(); mouseButton == (config::key_code - 0x100))
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
					mouse_delta_x = mouse_event->mouseInputX;
					break;
				}
                case RE::INPUT_EVENT_TYPE::kThumbstick:
                {
                    // Simulate mouse input based on thumbstick movement
                    auto thumbstick_event = reinterpret_cast<RE::ThumbstickEvent*>(input_event->AsIDEvent());
                    mouse_delta_x = static_cast<int32_t>(thumbstick_event->xValue * 10); // Scale the thumbstick value to simulate mouse movement
                    allow_rotate = true; // Allow rotation when thumbstick is moved
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
			auto player = RE::PlayerCharacter::GetSingleton();
			if (auto root = player->Get3D(false))
				root->local.rotate.ToEulerAnglesXYZ(angle);
		}
	}
	return RE::BSEventNotifyControl::kContinue;
}