set_project("logger")

-- version
set_version("0.0.1", { build = "%Y%m%d%H%M" })

-- set xmake min version
set_xmakever("2.9.3")

-- set warning all as error
--set_warnings("all", "error")

set_languages("c99", "c++20")

add_rules("mode.debug", "mode.release", "mode.valgrind")

if is_mode("release") then
    --set_optimize("smallest")
    if is_plat("windows") then
        add_ldflags("/LTCG")
    end
end

add_requires("spdlog", { configs = { shared = true } })

target("logger", function()
    --set_kind("$(kind)")
    set_kind("static")

    add_includedirs("include", { public = true })
    add_headerfiles("include/logger/*.hpp", "include/logger/**/*.hpp")

    add_files("src/*.cpp", "src/**/*.cpp")
    add_files("src/*.cppm", "src/**/*.cppm", { public = true })

    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
    add_configfiles("logger.config.cppm.in")
    add_includedirs("$(buildir)/$(plat)/$(arch)/$(mode)/logger.*.cppm", { public = true })

    if is_plat("windows") then
        add_defines("WIN32", "_WIN32")
    end

    add_packages("spdlog", { public = true })

    add_deps("common", { configs = { shared = true } })

    after_build(function(target)

    end)
end)
