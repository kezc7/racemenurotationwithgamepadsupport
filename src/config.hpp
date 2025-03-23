#pragma once

#define TOML_ENABLE_FORMATTERS 0
#define TOML_ENABLE_WINDOWS_COMPAT 0
#include <toml.hpp>

struct config
{
   static inline uint32_t key_code { 257 };
   static inline float min_rotate_speed = 3.f;
   static inline float max_rotate_speed = 12.f;

   static void load()
   {
       constexpr std::string_view config_path { "Data\\SKSE\\Plugins\\PlayerRotation.toml"sv };

       if (!std::filesystem::exists(config_path))
       {
           logger::warn("Could not find config at {}"sv, config_path);
           return;
       }

       try
       {
           auto tbl = toml::parse(config_path.data());
           key_code = toml::find_or(tbl, "KeyCode", key_code);
           min_rotate_speed = toml::find_or(tbl, "MinimumRotationSpeed", min_rotate_speed);
           max_rotate_speed = toml::find_or(tbl, "MaximumRotationSpeed", max_rotate_speed);
       } 
       catch (const toml::syntax_error& err)
       {
           logger::error("{}"sv, err.what());
           return;
       }
   }
};