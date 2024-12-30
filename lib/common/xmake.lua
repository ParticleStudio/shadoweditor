set_project("common")

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

target("common", function()
    set_kind("$(kind)")

    if is_plat("windows") then
        add_defines("WIN32", "_WIN32")
    end

    if is_kind("shared") then
        add_defines("COMMON_SHARED_LIB", "COMMON_EXPORT", { public = true })
    end

    add_includedirs("include", { public = true })
    add_headerfiles("include/common/*.hpp", "include/common/**/*.hpp")

    add_files("src/*.cpp", "src/**/*.cpp")
    add_files("src/*.cppm", "src/**/*.cppm", { public = true })

    set_configdir("$(buildir)/$(plat)/$(arch)/$(mode)")
    add_configfiles("common.config.cppm.in")
    add_files("$(buildir)/$(plat)/$(arch)/$(mode)/common.*.cppm", { public = true })

    after_build(function(target)

    end)
end)
