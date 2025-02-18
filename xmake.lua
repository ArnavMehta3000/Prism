set_project("Prism")

add_rules("mode.debug", "mode.release")

set_allowedmodes("debug", "release")
set_defaultmode("debug")
set_languages("c17", "cxx23")
set_policy("build.warning", true)
set_warnings("all", "extra")
set_policy("run.autobuild", true)

if is_mode("debug") then
	set_policy("preprocessor.linemarkers", true)
	add_defines("PRSIM_BUILD_DEBUG=1")
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

target("Prism")
	set_kind("binary")
	add_includedirs("Prism", { public = true })
	add_files("**.cpp")
	add_headerfiles("(Prism/**.h)", { install = true })
	add_packages("Elos")

	add_links("d3d11", "dxgi", "d3dcompiler")
target_end()