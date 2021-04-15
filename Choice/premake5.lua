project "Choice"
    kind "ConsoleApp"
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

        --ImGui
        "%{Dependency.imgui}/imgui.h",
        "%{Dependency.imgui}/imgui.cpp",
        "%{Dependency.imgui}/imconfig.h",
        "%{Dependency.imgui}/imgui_demo.cpp",
        "%{Dependency.imgui}/imgui_draw.cpp",
        "%{Dependency.imgui}/imgui_internal.h",
        "%{Dependency.imgui}/imgui_tables.cpp",
        "%{Dependency.imgui}/imgui_widgets.cpp",
        "%{Dependency.imgui}/imstb_rectpack.h",
        "%{Dependency.imgui}/imstb_textedit.h",
        "%{Dependency.imgui}/imstb_truetype.h",
        "%{Dependency.imgui}/backends/imgui_impl_glfw.h",
        "%{Dependency.imgui}/backends/imgui_impl_glfw.cpp",
        "%{Dependency.imgui}/backends/imgui_impl_opengl3.h",
        "%{Dependency.imgui}/backends/imgui_impl_opengl3.cpp",
        
        --ImGuiFileDialogs
        "%{Dependency.ImGuiFileDialog}/ImGuiFileDialog.h",
        "%{Dependency.ImGuiFileDialog}/ImGuiFileDialog.cpp",
        "%{Dependency.ImGuiFileDialog}/ImGuiFileDialogConfig.h",
        "%{Dependency.ImGuiFileDialog}/dirent/dirent.h",

        --ImGuizmo
        "%{Dependency.ImGuizmo}/ImGuizmo.h",
        "%{Dependency.ImGuizmo}/ImGuizmo.cpp"
    }

    includedirs
    {
        "src",
        "%{wks.location}/Quatation/src",
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
        "%{Dependency.ImGuizmo}"
    }

    links
    {
        "Quatation"
    }

    defines
    {
        "_CRT_SECURE_NO_WARNINGS"
    }

    filter "system:windows"
        defines "EXE"
    
    filter "configurations:Debug"
        defines "DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "RELEASE"
        runtime "Release"
        optimize "on"
