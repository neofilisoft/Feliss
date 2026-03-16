#pragma once

#include "scripting/ScriptEngine.h"

struct lua_State;

namespace Feliss {

class LuaScripting {
public:
    LuaScripting() = default;
    ~LuaScripting();

    bool init();
    void shutdown();

    lua_State* state() const { return m_state; }
    const std::string& lastError() const { return m_lastError; }

    bool LoadScript(const std::string& path);
    bool LoadBuffer(const std::string& code, const std::string& chunkName);
    bool CallFunction(const std::string& functionName,
                      const std::vector<ScriptValue>& args = {},
                      ScriptValue* out = nullptr);

    void pushValue(const ScriptValue& value);
    ScriptValue popValue();
    bool handleResult(int resultCode, const std::string& context);

private:
    lua_State*   m_state = nullptr;
    std::string  m_lastError;
};

} // namespace Feliss
