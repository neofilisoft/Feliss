#include "scripting/LuaScripting.h"

#include "core/Logger.h"

#include <type_traits>

#ifdef FELISS_LUA_ENABLED
extern "C" {
#  include <lua.h>
#  include <lauxlib.h>
#  include <lualib.h>
}
#endif

namespace Feliss {

LuaScripting::~LuaScripting() {
    shutdown();
}

bool LuaScripting::init() {
#ifdef FELISS_LUA_ENABLED
    shutdown();
    m_state = luaL_newstate();
    if (!m_state) {
        m_lastError = "luaL_newstate failed";
        FLS_ERROR("Lua", m_lastError);
        return false;
    }

    luaL_openlibs(m_state);
    FLS_INFOF("Lua", "Lua runtime initialized: " << LUA_RELEASE);
    return true;
#else
    m_lastError = "Lua not compiled in (missing FELISS_LUA_ENABLED)";
    FLS_WARN("Lua", m_lastError);
    return false;
#endif
}

void LuaScripting::shutdown() {
#ifdef FELISS_LUA_ENABLED
    if (m_state) {
        lua_close(m_state);
        m_state = nullptr;
        FLS_INFO("Lua", "Lua runtime closed");
    }
#endif
}

bool LuaScripting::LoadScript(const std::string& path) {
#ifdef FELISS_LUA_ENABLED
    if (!m_state) {
        m_lastError = "Lua runtime not initialized";
        return false;
    }

    int result = luaL_loadfile(m_state, path.c_str());
    if (result == LUA_OK) {
        result = lua_pcall(m_state, 0, LUA_MULTRET, 0);
    }
    return handleResult(result, "LoadScript(" + path + ")");
#else
    (void)path;
    return false;
#endif
}

bool LuaScripting::LoadBuffer(const std::string& code, const std::string& chunkName) {
#ifdef FELISS_LUA_ENABLED
    if (!m_state) {
        m_lastError = "Lua runtime not initialized";
        return false;
    }

    int result = luaL_loadbuffer(m_state, code.c_str(), code.size(), chunkName.c_str());
    if (result == LUA_OK) {
        result = lua_pcall(m_state, 0, LUA_MULTRET, 0);
    }
    return handleResult(result, "LoadBuffer(" + chunkName + ")");
#else
    (void)code;
    (void)chunkName;
    return false;
#endif
}

bool LuaScripting::CallFunction(const std::string& functionName,
                                const std::vector<ScriptValue>& args,
                                ScriptValue* out) {
#ifdef FELISS_LUA_ENABLED
    if (!m_state) {
        m_lastError = "Lua runtime not initialized";
        return false;
    }

    lua_getglobal(m_state, functionName.c_str());
    if (!lua_isfunction(m_state, -1)) {
        lua_pop(m_state, 1);
        m_lastError = "Lua global is not a function: " + functionName;
        FLS_WARN("Lua", m_lastError);
        return false;
    }

    for (const auto& arg : args) {
        pushValue(arg);
    }

    const int result = lua_pcall(m_state, static_cast<int>(args.size()), out ? 1 : 0, 0);
    if (!handleResult(result, "CallFunction(" + functionName + ")")) {
        return false;
    }

    if (out) {
        *out = popValue();
    }
    return true;
#else
    (void)functionName;
    (void)args;
    (void)out;
    return false;
#endif
}

void LuaScripting::pushValue(const ScriptValue& value) {
#ifdef FELISS_LUA_ENABLED
    std::visit([this](auto&& rawValue) {
        using T = std::decay_t<decltype(rawValue)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            lua_pushnil(m_state);
        } else if constexpr (std::is_same_v<T, bool>) {
            lua_pushboolean(m_state, rawValue ? 1 : 0);
        } else if constexpr (std::is_same_v<T, i64>) {
            lua_pushinteger(m_state, static_cast<lua_Integer>(rawValue));
        } else if constexpr (std::is_same_v<T, f64>) {
            lua_pushnumber(m_state, static_cast<lua_Number>(rawValue));
        } else if constexpr (std::is_same_v<T, std::string>) {
            lua_pushlstring(m_state, rawValue.c_str(), rawValue.size());
        }
    }, value);
#else
    (void)value;
#endif
}

ScriptValue LuaScripting::popValue() {
#ifdef FELISS_LUA_ENABLED
    if (!m_state || lua_gettop(m_state) == 0) {
        return std::monostate {};
    }

    ScriptValue result;
    switch (lua_type(m_state, -1)) {
        case LUA_TBOOLEAN:
            result = (lua_toboolean(m_state, -1) != 0);
            break;
        case LUA_TNUMBER:
            if (lua_isinteger(m_state, -1)) {
                result = static_cast<i64>(lua_tointeger(m_state, -1));
            } else {
                result = static_cast<f64>(lua_tonumber(m_state, -1));
            }
            break;
        case LUA_TSTRING:
            result = std::string(lua_tostring(m_state, -1));
            break;
        default:
            result = std::monostate {};
            break;
    }
    lua_pop(m_state, 1);
    return result;
#else
    return std::monostate {};
#endif
}

bool LuaScripting::handleResult(int resultCode, const std::string& context) {
#ifdef FELISS_LUA_ENABLED
    if (resultCode == LUA_OK) {
        return true;
    }

    const char* errorText = (m_state && lua_gettop(m_state) > 0) ? lua_tostring(m_state, -1) : nullptr;
    m_lastError = errorText ? errorText : "unknown Lua error";
    if (m_state && lua_gettop(m_state) > 0) {
        lua_pop(m_state, 1);
    }
    FLS_ERRORF("Lua", context << " failed: " << m_lastError);
    return false;
#else
    (void)resultCode;
    (void)context;
    return false;
#endif
}

} // namespace Feliss
