#pragma once

#include <cstdint>

// This state is deliberately independent of Skyrim/CommonLib so its transition
// rules can be unit tested without a running game.
struct RotationRuntimeState
{
	bool game_ready { false };
	bool loading_game { true };
	bool race_menu_open { false };
	bool angle_initialized { false };
	bool keyboard_mouse_held { false };
	bool gamepad_active { false };
	float mouse_delta_x { 0.f };
	float gamepad_delta_x { 0.f };
	std::uint8_t unsafe_frames_remaining { 0 };

	void reset_transient() noexcept
	{
		race_menu_open = false;
		angle_initialized = false;
		keyboard_mouse_held = false;
		clear_frame_input();
	}

	void clear_frame_input() noexcept
	{
		gamepad_active = false;
		mouse_delta_x = 0.f;
		gamepad_delta_x = 0.f;
	}

	void mark_data_loaded() noexcept
	{
		game_ready = true;
		loading_game = false;
		reset_transient();
		unsafe_frames_remaining = 1;
	}

	void begin_load() noexcept
	{
		game_ready = false;
		loading_game = true;
		reset_transient();
		unsafe_frames_remaining = 0;
	}

	void finish_load() noexcept
	{
		game_ready = true;
		loading_game = false;
		reset_transient();
		// Keep player/3D access disabled for a couple of Main::Update calls after
		// SKSE's post-load notification. The notification can precede final 3D
		// reconstruction by a frame.
		unsafe_frames_remaining = 2;
	}

	void begin_new_game() noexcept
	{
		game_ready = true;
		loading_game = false;
		reset_transient();
		unsafe_frames_remaining = 2;
	}

	void after_save() noexcept
	{
		reset_transient();
		// kSaveGame is the SKSE save notification; this barrier also prevents a
		// same-frame stale menu/input event from reaching player 3D code.
		unsafe_frames_remaining = 2;
	}

	[[nodiscard]] bool can_process_frame(bool main_active) const noexcept
	{
		return main_active && game_ready && !loading_game &&
			unsafe_frames_remaining == 0 && race_menu_open;
	}

	void advance_frame() noexcept
	{
		if (unsafe_frames_remaining > 0)
			--unsafe_frames_remaining;
	}
};
