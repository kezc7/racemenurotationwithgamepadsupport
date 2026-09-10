#include "config.hpp"
#include "event.hpp"

namespace
{
	[[nodiscard]] bool finite(const RE::NiPoint3& value) noexcept
	{
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
	}

	[[nodiscard]] bool address_in_segment(std::uintptr_t address, std::size_t size, REL::Segment::Name segment_name) noexcept
	{
		const auto segment = REL::Module::get().segment(segment_name);
		if (!segment.address() || size > segment.size())
			return false;

		const auto segment_end = segment.address() + segment.size();
		return address >= segment.address() && address <= segment_end - size;
	}

	[[nodiscard]] bool save_load_is_active() noexcept
	{
		// BGSSaveLoadGame exposes the engine's authoritative load/save flags. Check
		// the singleton relocation before calling its CommonLib accessor so a
		// runtime missing this optional safety relocation fails closed.
		const auto singleton_address = REL::RelocationID(516851, 403330).address();
		if (!singleton_address ||
			(!address_in_segment(singleton_address, sizeof(void*), REL::Segment::data) &&
				!address_in_segment(singleton_address, sizeof(void*), REL::Segment::rdata)))
			return true;

		const auto save_load = RE::BGSSaveLoadGame::GetSingleton();
		if (!save_load)
			return true;

		return save_load->GetSaveGameLoading() || save_load->GetSaveGameSaving() ||
			save_load->GetInitingForms() || save_load->GetDeferInitForms() ||
			save_load->GetPositioningPlayerCharacter();
	}

	[[nodiscard]] bool main_is_active(const RE::Main* main) noexcept
	{
		if (!main)
			return false;

		const auto& runtime_data = main->GetRuntimeData();
		return runtime_data.gameActive && !runtime_data.quitGame && !runtime_data.resetGame &&
			!runtime_data.fullReset && !save_load_is_active();
	}

	[[nodiscard]] bool valid_real_delta_address(std::uintptr_t address) noexcept
	{
		return address_in_segment(address, sizeof(float), REL::Segment::data) ||
			address_in_segment(address, sizeof(float), REL::Segment::rdata);
	}
}

void EventManager::reset_transient_state(std::string_view reason)
{
	runtime_state.reset_transient();
	angle = { 0.f, 0.f, 0.f };
	reported_missing_3d = false;
	reported_invalid_delta = false;
	logger::info("Transient rotation state reset ({})"sv, reason);
}

void EventManager::mark_input_loaded()
{
	// kInputLoaded is only the point at which the event sources can be wired.
	// Do not treat it as a loaded world; kDataLoaded/new-game/post-load establish
	// the actual runtime readiness state.
	logger::info("Input loaded; input and menu event sources are ready"sv);
}

void EventManager::on_skse_message(std::uint32_t message_type)
{
	switch (message_type)
	{
		case SKSE::MessagingInterface::kPreLoadGame:
			runtime_state.begin_load();
			angle = { 0.f, 0.f, 0.f };
			reported_missing_3d = false;
			reported_invalid_delta = false;
			logger::info("SKSE pre-load: rotation disabled and transient state reset"sv);
			break;

		case SKSE::MessagingInterface::kPostLoadGame:
			runtime_state.finish_load();
			angle = { 0.f, 0.f, 0.f };
			reported_missing_3d = false;
			reported_invalid_delta = false;
			logger::info("SKSE post-load: rotation re-armed after 3D settle barrier"sv);
			break;

		case SKSE::MessagingInterface::kNewGame:
			runtime_state.begin_new_game();
			angle = { 0.f, 0.f, 0.f };
			reported_missing_3d = false;
			reported_invalid_delta = false;
			logger::info("SKSE new game: transient state reset"sv);
			break;

		case SKSE::MessagingInterface::kSaveGame:
			runtime_state.after_save();
			angle = { 0.f, 0.f, 0.f };
			reported_missing_3d = false;
			reported_invalid_delta = false;
			logger::info("SKSE save notification: transient state reset"sv);
			break;

		case SKSE::MessagingInterface::kDeleteGame:
			runtime_state.reset_transient();
			angle = { 0.f, 0.f, 0.f };
			reported_missing_3d = false;
			reported_invalid_delta = false;
			logger::info("SKSE delete-game notification: transient state reset"sv);
			break;

		case SKSE::MessagingInterface::kDataLoaded:
			runtime_state.mark_data_loaded();
			angle = { 0.f, 0.f, 0.f };
			reported_missing_3d = false;
			reported_invalid_delta = false;
			logger::info("SKSE data loaded: rotation runtime ready"sv);
			break;

		default:
			break;
	}
}

bool EventManager::race_menu_is_open() const
{
	if (!runtime_state.race_menu_open)
		return false;

	const auto ui = RE::UI::GetSingleton();
	return ui && ui->IsMenuOpen(RE::RaceSexMenu::MENU_NAME);
}

RE::PlayerCharacter* EventManager::get_ready_player()
{
	const auto player = RE::PlayerCharacter::GetSingleton();
	if (!player || !player->IsInitialized() || player->IsDeleted())
		return nullptr;

	// This flag covers cell/world transitions where the actor exists but its
	// scene graph is being replaced.
	if (player->GetPlayerFlags().isLoading || !player->Is3DLoaded())
		return nullptr;

	return player;
}

void EventManager::update(RE::Main* main, std::uintptr_t real_delta_address)
{
	const auto main_active = main_is_active(main);
	if (!runtime_state.can_process_frame(main_active))
	{
		runtime_state.advance_frame();
		return;
	}

	if (!valid_real_delta_address(real_delta_address))
	{
		if (!reported_invalid_delta)
		{
			logger::error("Rotation disabled: real-time delta relocation is outside Skyrim data segments"sv);
			reported_invalid_delta = true;
		}
		runtime_state.clear_frame_input();
		runtime_state.advance_frame();
		return;
	}

	const auto real_delta = *reinterpret_cast<const float*>(real_delta_address);
	if (!std::isfinite(real_delta) || real_delta < 0.f)
	{
		if (!reported_invalid_delta)
		{
			logger::warn("Rotation skipped: real-time frame delta is invalid"sv);
			reported_invalid_delta = true;
		}
		runtime_state.clear_frame_input();
		runtime_state.advance_frame();
		return;
	}
	reported_invalid_delta = false;

	// Do not query the player, UI, or 3D until the runtime state and the actual
	// RaceMenu state both say this is a usable gameplay frame.
	if (!race_menu_is_open())
	{
		reset_transient_state("RaceMenu no longer open"sv);
		runtime_state.advance_frame();
		return;
	}

	const auto player = get_ready_player();
	if (!player)
	{
		if (!reported_missing_3d)
		{
			logger::debug("Rotation skipped: player or player 3D is unavailable"sv);
			reported_missing_3d = true;
		}
		runtime_state.clear_frame_input();
		runtime_state.advance_frame();
		return;
	}

	// Resolve the root afresh. No NiAVObject pointer survives a frame or a
	// load/cell transition.
	const auto root = player->Get3D(false);
	if (!root)
	{
		if (!reported_missing_3d)
		{
			logger::debug("Rotation skipped: player 3D root is unavailable"sv);
			reported_missing_3d = true;
		}
		runtime_state.clear_frame_input();
		runtime_state.advance_frame();
		return;
	}
	reported_missing_3d = false;

	if (!runtime_state.angle_initialized)
	{
		root->local.rotate.ToEulerAnglesXYZ(angle);
		if (!finite(angle))
		{
			logger::warn("Rotation skipped: player 3D root has a non-finite rotation"sv);
			reset_transient_state("non-finite player rotation"sv);
			runtime_state.advance_frame();
			return;
		}

		runtime_state.angle_initialized = true;
		// Capture on the first usable frame; begin rotating on a later frame.
		runtime_state.clear_frame_input();
		runtime_state.advance_frame();
		return;
	}

	const auto delta = std::min(real_delta, 0.25f);
	// Preserve the original controls: mouse movement rotates only while the
	// configured keyboard/mouse button is held; the right thumbstick is direct.
	const auto delta_x = runtime_state.keyboard_mouse_held ?
		runtime_state.mouse_delta_x :
		(runtime_state.gamepad_active ? runtime_state.gamepad_delta_x : 0.f);
	if (delta > 0.f && delta_x != 0.f && std::isfinite(delta_x) && finite(angle))
	{
		const auto direction = delta_x > 0.f ? -1.f : 1.f;
		const auto speed_t = std::min(std::abs(delta_x) / 360.f, 1.f);
		angle.z += direction * delta * std::lerp(config::min_rotate_speed, config::max_rotate_speed, speed_t);

		if (finite(angle))
		{
			// The root's local transform is the authoritative transform for this
			// preview. Do not force a recursive UpdateWorldData during Main::Update;
			// Skyrim will propagate it through its normal scene-graph update.
			root->local.rotate.SetEulerAnglesXYZ(angle);
		}
		else
		{
			logger::warn("Rotation skipped: calculated rotation became non-finite"sv);
			reset_transient_state("non-finite calculated rotation"sv);
		}
	}

	runtime_state.clear_frame_input();
	runtime_state.advance_frame();
}

RE::BSEventNotifyControl EventManager::ProcessEvent(RE::InputEvent* const* event, [[maybe_unused]] RE::BSTEventSource<RE::InputEvent*>* event_source)
{
	if (!event || !runtime_state.game_ready || runtime_state.loading_game ||
		!runtime_state.race_menu_open || !race_menu_is_open())
		return RE::BSEventNotifyControl::kContinue;

	for (auto input_event = *event; input_event; input_event = input_event->next)
	{
		switch (input_event->GetEventType())
		{
			case RE::INPUT_EVENT_TYPE::kButton:
			{
				const auto button_event = input_event->AsButtonEvent();
				// Gamepad rotation is gated by the thumbstick, not a button.
				if (!button_event || button_event->GetDevice() == RE::INPUT_DEVICE::kGamepad)
					continue;

				bool is_rotation_button = false;
				switch (button_event->GetDevice())
				{
					case RE::INPUT_DEVICE::kKeyboard:
						is_rotation_button = button_event->GetIDCode() == config::key_code;
						break;
					case RE::INPUT_DEVICE::kMouse:
						is_rotation_button = config::key_code >= 0x100 &&
							button_event->GetIDCode() == config::key_code - 0x100;
						break;
					default:
						break;
				}

				if (is_rotation_button)
					runtime_state.keyboard_mouse_held = button_event->IsPressed();
				continue;
			}

			case RE::INPUT_EVENT_TYPE::kMouseMove:
			{
				if (const auto mouse_event = input_event->AsMouseMoveEvent())
					runtime_state.mouse_delta_x = static_cast<float>(mouse_event->mouseInputX);
				break;
			}

			case RE::INPUT_EVENT_TYPE::kThumbstick:
			{
				if (const auto thumbstick_event = input_event->AsThumbstickEvent();
					thumbstick_event && thumbstick_event->IsRight())
				{
					runtime_state.gamepad_delta_x = thumbstick_event->xValue * 360.f;
					runtime_state.gamepad_active =
					std::isfinite(runtime_state.gamepad_delta_x) && runtime_state.gamepad_delta_x != 0.f;
				}
				break;
			}

			default:
				break;
		}
	}
	return RE::BSEventNotifyControl::kContinue;
}

RE::BSEventNotifyControl EventManager::ProcessEvent(const RE::MenuOpenCloseEvent* event, [[maybe_unused]] RE::BSTEventSource<RE::MenuOpenCloseEvent>* event_source)
{
	if (!event || event->menuName != RE::RaceSexMenu::MENU_NAME)
		return RE::BSEventNotifyControl::kContinue;

	if (event->opening)
	{
		if (!runtime_state.game_ready || runtime_state.loading_game)
		{
			reset_transient_state("RaceMenu opened during lifecycle transition"sv);
			return RE::BSEventNotifyControl::kContinue;
		}

		runtime_state.reset_transient();
		angle = { 0.f, 0.f, 0.f };
		runtime_state.race_menu_open = true;
		reported_missing_3d = false;
		logger::info("RaceMenu opened; deferring player 3D capture to next usable frame"sv);
	}
	else
	{
		reset_transient_state("RaceMenu closed"sv);
	}

	return RE::BSEventNotifyControl::kContinue;
}
