#include "hooks.hpp"
#include "event.hpp"

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
    stl::write_thunk_call<Main_Update>(REL::RelocationID(35551, 36544).address() + REL::Relocate(0x11F, 0x160));
}