set_project("engine")

-- version
set_version("0.0.1", { build = "%Y%m%d%H%M" })

-- set xmake min version
set_xmakever("2.9.3")

-- set warning all as error
--set_warnings("all", "error")

set_languages("c99", "c++20")

add_rules("mode.debug", "mode.release", "mode.valgrind")

set_encodings("utf-8")

if is_mode("release") then
    --set_optimize("smallest")
    if is_plat("windows") then
        add_ldflags("/LTCG")
    end
end

add_requires("spdlog")

target("engine", function()
    set_kind("binary")

    add_includedirs("include")
    add_headerfiles("include/*.hpp", "include/**/*.hpp")

    add_files("src/*.cpp", "src/*.cpp")
    add_files("src/*.cpp", "src/*.cpp", { public = true })

    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
    add_configfiles("engine.config.cppm.in")
    add_files("$(buildir)/$(plat)/$(arch)/$(mode)/engine.*.cppm", { public = true })

    if is_plat("windows") then
        add_defines("WIN32", "_WIN32")
    end

    add_defines(
            "COMMON_SHARED_LIB",
            "LOGGER_SHARED_LIB",
            "BEHAVIORTREE_SHARED_LIB",
            "JSENGINE_SHARED_LIB"
    )

    add_deps("common", { configs = { shared = true } })
    add_deps("logger", { configs = { shared = true } })
    add_deps("behaviortree", { configs = { shared = true } })
    add_deps("jsengine", { configs = { shared = true } })

    after_build(function(target)

    end)
end)

add_requires("gtest")
target("engine_test", function()
    add_files("test/*.cpp", "src/**/*.cpp")

    add_packages("gtest")
end)
