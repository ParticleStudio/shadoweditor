set_project("JSEngine")

-- version
set_version("0.0.1", { build = "%Y%m%d%H%M" })

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

add_requires("quickjs", { configs = { shared = true } })

target("JSEngine", function()
    set_kind("shared")

    add_includedirs("include", { public = true, prefixdir = "jsengine" })
    add_headerfiles("jsengine/(*.h)", "jsengine/(*.hpp)")

    add_includedirs("../common/include", { public = true, prefixdir = "common" })
    add_headerfiles("common/(*.h)", "common/(*.hpp)")

    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
    add_configfiles("jsengine.config.h.in")
    add_includedirs("$(buildir)/$(plat)/$(arch)/$(mode)", { public = true })

    add_files("src/*.cpp", "src/*.hpp", "src/*.cppm", "src/**/*.cpp", "src/**/*.hpp", "src/**/*.cppm")

    add_defines("SHARED_LIB")
    if is_plat("windows") then
        add_defines("WIN32", "_WIN32", "DLLEXPORT")
    end

    add_packages("quickjs")

    after_build(function(target)
        --local outdir = "$(buildir)/$(plat)/$(arch)/$(mode)"
        --os.cp("conf", outdir)
        --os.cp("script", outdir)
    end)
end)
