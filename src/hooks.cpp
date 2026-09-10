#include "hooks.hpp"
#include "event.hpp"

namespace
{
	// Address Library gives us the address of Skyrim's real-time frame delta.
	// Keep it as an integer until EventManager has passed all lifecycle gates.
	std::uintptr_t g_delta_time_real_time { 0 };
	bool g_hook_installed { false };

	[[nodiscard]] bool address_in_segment(std::uintptr_t address, std::size_t size, REL::Segment::Name segment_name) noexcept
	{
		const auto segment = REL::Module::get().segment(segment_name);
		if (!segment.address() || size > segment.size())
			return false;

		const auto segment_end = segment.address() + segment.size();
		return address >= segment.address() && address <= segment_end - size;
	}

	struct Main_Update
	{
		static void thunk(RE::Main* self, float delta)
		{
			func(self, delta);

			// EventManager performs the lifecycle/menu/player checks before it
			// dereferences the real-time delta or touches any game object.
			if (g_hook_installed)
				EVENT_MANAGER.update(self, g_delta_time_real_time);
		}

		static inline REL::Relocation<decltype(thunk)> func;
	};
}

bool hooks::install()
{
	if (REL::Module::IsVR())
	{
		logger::error("Skyrim VR is unsupported; Main::Update hook will not be installed"sv);
		return false;
	}

	const auto delta_address = REL::RelocationID(523661, 410200).address();
	const auto update_address = REL::RelocationID(35551, 36544).address();
	const auto update_offset = static_cast<std::uintptr_t>(REL::Relocate(0x11F, 0x160));
	const auto patch_address = update_address ? update_address + update_offset : 0;

	if (!delta_address ||
		(!address_in_segment(delta_address, sizeof(float), REL::Segment::data) &&
			!address_in_segment(delta_address, sizeof(float), REL::Segment::rdata)) ||
		!update_address || !address_in_segment(patch_address, 5, REL::Segment::textx))
	{
		logger::error("Could not validate Main update relocations; plugin will not load"sv);
		logger::error("Skyrim runtime: {}, delta=0x{:X}, update=0x{:X}, patch=0x{:X}"sv,
			REL::Module::get().version().string(), delta_address, update_address, patch_address);
		return false;
	}

	g_delta_time_real_time = delta_address;
	logger::info("Installing Main::Update hook for Skyrim runtime {} at 0x{:X}"sv,
		REL::Module::get().version().string(), patch_address);
	stl::write_thunk_call<Main_Update>(patch_address);
	g_hook_installed = true;
	logger::info("Main::Update hook installed; player 3D work remains lifecycle-gated"sv);
	return true;
}
