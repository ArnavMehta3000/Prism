set_project("Prism")

includes("**/xmake.lua")
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", { outputdir = "build" })

set_allowedmodes("debug", "release")
set_defaultmode("debug")
set_languages("c17", "cxx23")
set_policy("build.warning", true)
set_warnings("all", "extra")
set_policy("run.autobuild", true)

if is_mode("debug") then
	set_policy("preprocessor.linemarkers", true)
	add_defines("PRISM_BUILD_DEBUG=1", "_DEBUG")
	set_symbols("debug", "edit")
end

if is_mode("release") then
	set_symbols("hidden")
	add_defines("PRISM_BUILD_DEBUG=0")
	set_strip("all")
end

add_defines("UNICODE", "_UNICODE", "NOMINMAX", "NOMCX", "NOSERVICE", "NOHELP", "WIN32_LEAN_AND_MEAN")
add_tests("CompileSuccess", { build_should_pass = true, group = "Compilation" })

set_runtimes(is_mode("debug") and "MTd" or "MT")

add_requires("Elos 57332c984b0a02c4dc6921583057161f14ae66d5")
add_requires("imgui 2d403a16144070a4cb46bb124318b20141e83cb4", { configs = { dx11 = true, win32 = true } })
add_requires("cxxopts", "directxtk", "assimp")
set_policy("build.c++.modules", true)

target("ShaderCompiler")
	set_kind("binary")
	set_default(false)

	add_files("ShaderCompiler/**.ixx")
	add_files("ShaderCompiler/**.cpp")

	add_links("d3dcompiler", { public = true })
	add_packages("cxxopts")

	set_policy("build.fence", true)
target_end()

target("Prism")
	set_kind("binary")

	add_includedirs("Prism", { public = true })
	add_files("Prism/**.cpp")
	add_files("Prism/**.ixx")
	add_files("Shaders/**.hlsl", { install = true })
	add_headerfiles("(Prism/**.h)", { install = true })

	add_packages("Elos", "directxtk", "assimp", "imgui")
	add_deps("ShaderCompiler")

	add_rules("CompileHLSL")
	add_links("d3d11", "dxgi", "dxguid", "uuid")

	on_config(function(target)
		local assetsPath = path.join(os.projectdir(), "Assets")
		assetsPath = assetsPath:gsub("\\", "/")
		local defineValue = "PRISM_ASSETS_PATH=\"" .. assetsPath .. "\""
		target:add("defines", defineValue)
	end)
target_end()
