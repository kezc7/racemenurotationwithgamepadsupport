#include "hooks.hpp"
#include "event.hpp"

// Real-time frame delta — menus pause game time, so the game-time delta would be 0
static float* g_deltaTimeRealTime = nullptr;

struct Main_Update
{
    static void thunk(RE::Main* self, float delta)
    {
        func(self, delta);
        EVENT_MANAGER.update(*g_deltaTimeRealTime);
    }

    static inline REL::Relocation<decltype(thunk)> func;
};

void hooks::install()
{
    g_deltaTimeRealTime = reinterpret_cast<float*>(REL::RelocationID(523661, 410200).address());
    stl::write_thunk_call<Main_Update>(REL::RelocationID(35551, 36544).address() + REL::Relocate(0x11F, 0x160));
}
