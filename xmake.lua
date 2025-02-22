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
add_requires("Elos")

add_requires("cxxopts")

target("ShaderCompiler")
	set_kind("binary")

	add_includedirs("ShaderCompiler", { public = true })
	add_files("ShaderCompiler/**.cpp")
	add_headerfiles("(ShaderCompiler/**.h)", { install = true })

	add_links("d3dcompiler")
	add_packages("cxxopts")

	set_policy("build.fence", true)
target_end()

target("Prism")
	set_kind("binary")

	add_includedirs("Prism", { public = true })
	add_files("Prism/**.cpp")
	add_files("Shaders/**.hlsl", {install = true })
	add_headerfiles("(Prism/**.h)", { install = true })

	add_packages("Elos")
	add_deps("ShaderCompiler")

	add_rules("CompileHLSL")
	add_links("d3d11", "dxgi", "dxguid", "uuid")
target_end()
