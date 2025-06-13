#if defined(DM_PLATFORM_HTML5)

#include <dmsdk/sdk.h>
#include <string>
#include <unordered_map>

namespace {

using CallbackMap = std::unordered_map<std::string, dmScript::LuaCallbackInfo*>;

static CallbackMap s_listeners;

// Forward declaration
static void DispatchToLua(const char* ev, const char* data);

// C wrapper callable from JS
extern "C" {
    void LuaBridge_Dispatch(const char* ev, const char* data) {
        DispatchToLua(ev, data);
    }
}

static void DispatchToLua(const char* ev, const char* data) {
    auto it = s_listeners.find(ev);
    if (it == s_listeners.end()) return;

    dmScript::LuaCallbackInfo* cbinfo = it->second;
    if (!dmScript::IsCallbackValid(cbinfo))
        return;

    lua_State* L = cbinfo->m_L;
    DM_LUA_STACK_CHECK(L, 0);

    if (dmScript::SetupCallback(cbinfo)) {
        lua_pushstring(L, ev);
        lua_pushstring(L, data);
        dmScript::PCall(L, 3, 0);
        dmScript::TeardownCallback(cbinfo);
    }
}

// ---------- Lua: emit(event, json_string) -------------
static int Lua_Emit(lua_State* L)
{
    const char* ev   = luaL_checkstring(L, 1);
    const char* data = luaL_optstring(L, 2, "{}");

    // Forward to JS side
    EM_ASM_({
        LuaBridge_JS_Emit($0, $1);
    }, ev, data);

    return 0;
}

// ---------- Lua: on(event, function) ------------------
static int Lua_On(lua_State* L)
{
    const char* ev = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    // Remove old listener if exists
    auto it = s_listeners.find(ev);
    if (it != s_listeners.end()) {
        dmScript::DestroyCallback(it->second);
        s_listeners.erase(it);
    }

    dmScript::LuaCallbackInfo* cbinfo = dmScript::CreateCallback(L, 2);
    s_listeners[ev] = cbinfo;

    return 0;
}

// ---------- Extension lifecycle ----------------------
static dmExtension::Result AppInitialize(lua_State*, dmExtension::AppParams*)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalize(lua_State*, dmExtension::AppParams*)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result Initialize(lua_State* L, dmExtension::Params*)
{
    luaL_Reg funcs[] = {
        {"emit", Lua_Emit},
        {"on",   Lua_On},
        {0,0}
    };

    luaL_register(L, "luajs", funcs);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result Finalize(lua_State*, dmExtension::Params*)
{
    for (auto& kv : s_listeners)
        dmScript::DestroyCallback(kv.second);
    s_listeners.clear();
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(LUAJSBRIDGE, "LuaJSBridge",
    AppInitialize, AppFinalize, Initialize, 0, 0, Finalize)

} // end anonymous namespace
#endif // DM_PLATFORM_HTML5
