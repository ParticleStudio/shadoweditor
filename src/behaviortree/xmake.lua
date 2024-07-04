set_project("BehaviorTree")

add_configfiles("config.h.in")

-- version
set_version("0.0.1", {build = "%Y%m%d%H%M"})

-- set xmake min version
set_xmakever("2.9.3")

-- set warning all as error
--set_warnings("all", "error")

set_languages("c99", "c++20")

add_rules("mode.debug", "mode.release")

if is_mode("release") then
    set_optimize("smallest")
    if is_plat("windows") then
        add_ldflags("/LTCG")
    end
end

add_requires("lexy")
add_requires("conan::minicoro/0.1.3", {alias = "minicoro"})
add_requires("conan::wildcards/1.4.0", {alias = "wildcards"})

target("BehaviorTree", function()
    set_kind("shared")

    includes("lib/**/xmake.lua", "src/**/xmake.lua")

    add_includedirs("/include")
    add_files("src/*.cpp", "src/*.hpp", "src/*.cppm")

    add_defines("BEHAVIORTREE_LIBRARY_VERSION")
    add_defines("SHARED_LIB")
    if is_plat("windows") then
        add_defines("WIN32", "_WIN32", "DLLEXPORT")
    end

    add_packages("lexy")
    add_packages("minicoro")
    add_packages("wildcards")

    after_build(function(target)
        --local outdir = "$(buildir)/$(plat)/$(arch)/$(mode)"
        --os.cp("conf", outdir)
        --os.cp("script", outdir)
    end)
end)
