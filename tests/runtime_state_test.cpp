#include "runtime_state.hpp"

#include <cassert>

int main()
{
	RotationRuntimeState state;
	assert(!state.can_process_frame(true));

	state.mark_data_loaded();
	state.race_menu_open = true;
	state.angle_initialized = true;
	assert(!state.can_process_frame(true));
	state.advance_frame();
	assert(state.can_process_frame(true));

	state.keyboard_mouse_held = true;
	state.gamepad_active = true;
	state.mouse_delta_x = 2.f;
	state.gamepad_delta_x = 3.f;
	state.clear_frame_input();
	assert(state.keyboard_mouse_held);
	assert(!state.gamepad_active);
	assert(state.mouse_delta_x == 0.f);
	assert(state.gamepad_delta_x == 0.f);

	state.finish_load();
	assert(!state.can_process_frame(true));
	assert(!state.race_menu_open);
	assert(!state.angle_initialized);
	state.advance_frame();
	state.advance_frame();
	assert(!state.can_process_frame(true));

	state.race_menu_open = true;
	state.angle_initialized = true;
	state.begin_load();
	assert(!state.game_ready);
	assert(state.loading_game);
	assert(!state.can_process_frame(true));

	state.finish_load();
	state.advance_frame();
	state.advance_frame();
	assert(state.game_ready);
	assert(!state.loading_game);
	assert(!state.race_menu_open);

	state.begin_new_game();
	assert(!state.can_process_frame(true));
	state.advance_frame();
	state.advance_frame();
	assert(!state.race_menu_open);

	state.race_menu_open = true;
	state.angle_initialized = true;
	state.after_save();
	assert(!state.can_process_frame(true));
	assert(!state.race_menu_open);
	return 0;
}
