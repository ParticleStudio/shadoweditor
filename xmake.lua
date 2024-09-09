set_project("behaviortree_editor")

-- version
set_version("0.0.1", { build = "%Y%m%d%H%M" })

-- set xmake min version
set_xmakever("2.9.3")

-- set warning all as error
--set_warnings("all", "error")

set_languages("c99", "c++20")

includes("lib/**/xmake.lua", "src/**/xmake.lua")
