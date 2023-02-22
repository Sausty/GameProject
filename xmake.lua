--
-- Author: Amélie Heinrich
-- Company: Amélie Games
-- License: MIT
-- Create Time: 01/01/2023 18:16
--

add_rules("mode.debug", "mode.release")

includes("external", "tools")

target("Game")
    set_languages("c11", "c++20")
    set_rundir(".")
    add_deps("ImGui", "dr_libs", "stb")
    add_files("src/*.cpp", "src/cameras/*.cpp", "src/gui/*.cpp", "src/renderer/*.cpp", "src/renderer/passes/*.cpp", "src/scene/*.cpp", "src/systems/*.cpp")
    add_headerfiles("src/**.hpp")
    add_includedirs("src", "external", { public = true })
    add_linkdirs("bin/")

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
    end

    if is_mode("release") then
        set_symbols("hidden")
        set_optimize("fastest")
        set_strip("all")
    end

    if is_plat("windows") then
        add_deps("D3D12MA")
        add_syslinks("user32", "dsound", "gdi32", "kernel32", "d3d12", "d3dcompiler", "dxgi", "assimp-vc143-mtd")
        add_files("src/apu/dsound/*.cpp", "src/gpu/dx12/*.cpp", "src/gui/dx12/*.cpp", "src/systems/windows/*.cpp", "src/windows/*.cpp")
        add_files("src/main_win32.cpp")
    end
    -- TODO(amelie.h): MacOS
    -- TODO(amelie.h): Linux
    -- TODO(amelie.h): Nintendo Switch? :flushed:
