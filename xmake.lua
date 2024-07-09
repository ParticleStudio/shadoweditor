set_project("ShadowEditor")

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

includes("src/**/xmake.lua")
