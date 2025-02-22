task("CompileShaders")
set_menu {
        usage = "xmake CompileShaders [file]",
        description = "Compile HLSL shader files",
        options =
        {
            { nil, "file", "v", nil, "The shader file to compile" }
        }
    }
    on_run(function ()
        import("core.project.project")
        import("core.base.option")
        import("core.base.json")

        -- Get the source file path relative to Shaders directory
        local source_file = option.get("file")
        if not source_file then
                    raise("No shader file specified!")
        end

        -- Locate Prism
        local prism_target = project.target("Prism")
        if not prism_target then
            raise("Target 'Prism' not found!")
        end

        -- Locate shader compiler
        local compiler_target = project.target("ShaderCompiler")
        if not compiler_target then
        	raise("Target 'ShaderCompiler' not found!")
        end

        -- Ensure compiler is built
        if not os.exists(compiler_target:targetfile()) then
        	raise("ShaderCompiler not built!")
        end

        -- Locate shader manifest
        local manifest_path = path.join(os.projectdir(), "Shaders", "ShaderManifest.json")
        if not os.exists(manifest_path) then
        	raise("ShaderManifest.json not found. Expected dir: ".. manifest_path)
        end

        -- Load manifest data
        local manifest_data = json.loadfile(manifest_path)
        local manifest_common_defines = manifest_data["common_defines"]
        local manifest_shaders = manifest_data["shaders"]

        -- Get output directory
        local output_dir = path.join(path.absolute(prism_target:targetdir()), "Shaders")
        if not os.isdir(output_dir) then
            os.mkdir(output_dir)
        end

        -- Find the shader in manifest
        local shader_config = nil
        for _, shader in ipairs(manifest_shaders) do
            -- Normalize paths by converting all separators to forward slashes
            local relative = path.relative(source_file, "Shaders"):gsub("\\", "/")
            local shader_path = shader.file:gsub("\\", "/")

            if relative == shader_path then
                shader_config = shader
                break
            end
        end

        if not shader_config then
            raise(string.format("Shader (%s) not found in manifest! But exists in directory", source_file))
        end

        local invoke_compiler = function (file, type, profile, entry, defines)
        	-- Get the directory path relative to the Shaders folder
            local shader_dir = path.directory(file)
            local relative_path = ""

            -- Only process relative path if the shader is not in root Shaders directory
            if shader_dir ~= "Shaders" then
                relative_path = path.relative(shader_dir, "Shaders")
            end

            -- Create the output directory with nested structure if needed
            local output_subdir = output_dir
            if relative_path ~= "" then
                output_subdir = path.join(output_dir, relative_path)
                if not os.isdir(output_subdir) then
                    os.mkdir(output_subdir)
                end
            end

            -- Construct the output filename with shader type suffix
            local base_name = path.basename(file)
            local shader_suffix = "_" .. string.upper(type)
            local out_filename = base_name .. shader_suffix .. ".hlsl"
            local out_file = path.join(output_subdir, out_filename)

            print(string.format("Processing shader (%s) as [%s]", file, string.upper(type)))
            --print(string.format("Output path: %s", out_file))

            local args =
            {
                "--in", file,
                "--type", type,
                "--profile", profile,
                "--entry", entry,
                "--out", out_file
            }

            -- Add each define as a separate argument
            for _, define in ipairs(defines) do
                table.insert(args, "--define")
                table.insert(args, define)
            end

            os.execv(compiler_target:targetfile(), args)
        end

        -- Compile each stage for this shader
        for stage, stage_data in pairs(shader_config.stages) do
            -- Capture all defines
            local all_defines = {}
            for _, define in ipairs(manifest_common_defines or {}) do
                table.insert(all_defines, define)
            end

            for _, define in ipairs(stage_data.defines) do
                table.insert(all_defines, define)
            end

            -- Invoke the compiler for this stage
            invoke_compiler(
                source_file,
                string.upper(stage),
                stage_data.profile,
                stage_data.entry,
                all_defines
            )
        end

    end)
task_end()

rule("CompileHLSL")
    set_extensions(".hlsl")

    on_build_file(function (target, sourcefile, opt)
    	import("core.base.task")
        import("core.project.depend")



        depend.on_changed(function ()
        	task.run("CompileShaders", {file = sourcefile})
        end, { files = sourcefile })

    end)
rule_end()
