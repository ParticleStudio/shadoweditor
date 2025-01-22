set_project("network")

-- version
set_version("0.0.1", { build = "%Y%m%d%H%M" })

-- set xmake min version
set_xmakever("2.9.3")

-- set warning all as error
--set_warnings("all", "error")

set_languages("c17", "cxx20") -- https://xmake.io/#/zh-cn/manual/project_target?id=targetset_languages

add_rules("mode.debug", "mode.release", "mode.valgrind")

set_encodings("utf-8")

if is_mode("release") then
    --set_optimize("smallest")
    if is_plat("windows") then
        add_ldflags("/LTCG")
    end
end

target("network", function()
    set_kind("$(kind)")

    if is_plat("windows") then
        add_defines("WIN64", "_WIN64", "_WIN32_WINNT=0x0601")
    end

    if is_kind("shared") then
        add_defines("NETWORK_SHARED_LIB", "NETWORK_EXPORT", { public = true })
    end

    add_includedirs("include", { public = true })
    add_headerfiles("include/network/*.hpp", "include/network/**/*.hpp")

    add_files("src/*.cpp", "src/**/*.cpp")
    add_files("src/*.cppm", "src/**/*.cppm", { public = true })

    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
    add_configfiles("network.config.cppm.in")
    add_files("$(buildir)/$(plat)/$(arch)/$(mode)/network.*.cppm", { public = true })

    add_deps("common", { configs = { shared = true } })

    after_build(function(target)

    end)
end)
