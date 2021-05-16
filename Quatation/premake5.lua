project "Quatation"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    systemversion "latest"

    targetdir(bindir)
    objdir(objbindir)
    debugdir "%{wks.location}"

    files
    {
        "src/**.h",
        "src/**.cpp",

        --Glfw
        "%{Dependency.glfw}/include/GLFW/glfw3.h",
        "%{Dependency.glfw}/include/GLFW/glfw3native.h",
        "%{Dependency.glfw}/src/internal.h", 
        "%{Dependency.glfw}/src/mappings.h", 
        "%{Dependency.glfw}/src/context.c", 
        "%{Dependency.glfw}/src/init.c", 
        "%{Dependency.glfw}/src/input.c", 
        "%{Dependency.glfw}/src/monitor.c",
        "%{Dependency.glfw}/src/vulkan.c", 
        "%{Dependency.glfw}/src/window.c",

        --Glad
        "%{Dependency.glad}/include/glad/glad.h",
        "%{Dependency.glad}/include/KHR/khrplatform.h",
        "%{Dependency.glad}/src/glad.c",

        --cgltf
        "%{Dependency.cgltf}/cgltf.h",

        --stb_image
        "%{Dependency.stb_image}/stb_image.h",
        "%{Dependency.stb_image}/stb_image.cpp",

        --tinyexr
        "%{Dependency.tinyexr}/tinyexr.h",
        "%{Dependency.tinyexr}/tinyexr.cpp",

        --SPIRV_Cross
        "%{Dependency.SPIRV_Cross}/GLSL.std.450.h",
        "%{Dependency.SPIRV_Cross}/spirv_common.hpp",
		"%{Dependency.SPIRV_Cross}/spirv_cross_containers.hpp",
		"%{Dependency.SPIRV_Cross}/spirv_cross_error_handling.hpp",
		"%{Dependency.SPIRV_Cross}/spirv.hpp",
		"%{Dependency.SPIRV_Cross}/spirv_cross.hpp",
		"%{Dependency.SPIRV_Cross}/spirv_cross.cpp",
		"%{Dependency.SPIRV_Cross}/spirv_parser.hpp",
		"%{Dependency.SPIRV_Cross}/spirv_parser.cpp",
		"%{Dependency.SPIRV_Cross}/spirv_cross_parsed_ir.hpp",
		"%{Dependency.SPIRV_Cross}/spirv_cross_parsed_ir.cpp",
		"%{Dependency.SPIRV_Cross}/spirv_cfg.hpp",
        "%{Dependency.SPIRV_Cross}/spirv_cfg.cpp",
        "%{Dependency.SPIRV_Cross}/spirv.h",
		"%{Dependency.SPIRV_Cross}/spirv_cross_c.cpp",
		"%{Dependency.SPIRV_Cross}/spirv_cross_c.h",
        "%{Dependency.SPIRV_Cross}/spirv_glsl.cpp",
		"%{Dependency.SPIRV_Cross}/spirv_glsl.hpp",
        "%{Dependency.SPIRV_Cross}/spirv_cpp.cpp",
		"%{Dependency.SPIRV_Cross}/spirv_cpp.hpp",
        "%{Dependency.SPIRV_Cross}/spirv_msl.cpp",
		"%{Dependency.SPIRV_Cross}/spirv_msl.hpp",
        "%{Dependency.SPIRV_Cross}/spirv_hlsl.cpp",
        "%{Dependency.SPIRV_Cross}/spirv_hlsl.hpp",
        "%{Dependency.SPIRV_Cross}/spirv_reflect.cpp",
		"%{Dependency.SPIRV_Cross}/spirv_reflect.hpp",
        "%{Dependency.SPIRV_Cross}/spirv_cross_util.cpp",
		"%{Dependency.SPIRV_Cross}/spirv_cross_util.hpp"
    }

    includedirs
    {
        "src",
        "%{wks.location}/Choice/src",
        "%{Dependency.glfw}/include",
        "%{Dependency.glad}/include",
        "%{Dependency.glm}",
        "%{Dependency.cgltf}",
        "%{Dependency.imgui}",
        "%{Dependency.ImGuiFileDialog}",
        "%{Dependency.compressonator}",
        "%{Dependency.stb_image}",
        "%{Dependency.gli}",
        "%{Dependency.tinyexr}",
        "%{Dependency.filesystem}",
        "%{Dependency.ImGuizmo}",
        "%{Dependency.SPIRV_Cross}"
    }

    libdirs
    {
        "%{Dependency.compressonator}/bin"
    }

    links
    {
        "CMP_Framework.lib"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }

    filter "system:windows"
        defines
        {
            "EXE",
            "_GLFW_WIN32"
        }

        files
        {
            "%{Dependency.glfw}/src/win32_platform.h",
            "%{Dependency.glfw}/src/win32_joystick.h", 
            "%{Dependency.glfw}/src/wgl_context.h",
            "%{Dependency.glfw}/src/egl_context.h",           
            "%{Dependency.glfw}/src/osmesa_context.h",
            "%{Dependency.glfw}/src/win32_init.c",
            "%{Dependency.glfw}/src/win32_joystick.c",
            "%{Dependency.glfw}/src/win32_monitor.c ",               
            "%{Dependency.glfw}/src/win32_time.c",
            "%{Dependency.glfw}/src/win32_thread.c",
            "%{Dependency.glfw}/src/win32_window.c",
            "%{Dependency.glfw}/src/wgl_context.c",              
            "%{Dependency.glfw}/src/egl_context.c",
            "%{Dependency.glfw}/src/osmesa_context.c"
        }
    
    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"