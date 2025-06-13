local json = require("libs.json")     -- adapt path if you use a different json lib
local ext  = nil
local stub = { emit = function() end, on = function() end }

if html5 and html5.run then
    pcall(function() ext = require "luajs" end)
end
ext = ext or stub    -- fallback on non-HTML5 builds

local M = {}

function M.emit(event, payload)
    assert(type(event) == "string", "event name must be a string")
    if type(payload) == "table" then
        payload = json.encode(payload)
    end
    ext.emit(event, payload or "{}")
end

function M.on(event, fn)
    assert(type(event) == "string", "event name must be a string")
    assert(type(fn) == "function", "listener must be function")
    ext.on(event, fn)
end

return M
