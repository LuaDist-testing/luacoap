-- This file was automatically generated for the LuaDist project.

package = "luacoap"
version = "0.1-0"

-- LuaDist source
source = {
  tag = "0.1-0",
  url = "git://github.com/LuaDist-testing/luacoap.git"
}
-- Original source
-- source = {
--   url = "git://github.com/alfonsoros88/luacoap",
--   branch = "luarocks",
--   tag = "v0.1",
-- }

description = {
   summary = "Coap Client in Lua.",
   detailed = [[ Just a CoAP client for lua now, eventually a server in the future. ]],
   license = "MIT/X11"
}

dependencies = {
   "lua >= 5.1, < 5.4"
}

build = {
  type = "cmake"
}