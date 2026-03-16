#include "scripting/CSharpBridge.h"
#include "core/Engine.h"
#include "core/Logger.h"

#include <algorithm>

#ifdef FELISS_CSHARP_ENABLED
#  include <mono/jit/jit.h>
#  include <mono/metadata/assembly.h>
#  include <mono/metadata/mono-config.h>
#  include <mono/metadata/object.h>
#  include <mono/metadata/threads.h>
#  include <mono/metadata/attrdefs.h>
#  include <mono/metadata/class.h>
#  include <mono/metadata/debug-helpers.h>
#  include <mono/metadata/metadata.h>
#endif

namespace Feliss {

#ifdef FELISS_CSHARP_ENABLED
namespace {

struct MonoInvokeArgs {
    std::vector<i64> integers;
    std::vector<f64> numbers;
    std::vector<char> booleans;
    std::vector<MonoString*> strings;
    std::vector<void*> params;
};

const char* monoTypeName(MonoObject* value) {
    if (!value) {
        return "<null>";
    }
    MonoClass* klass = mono_object_get_class(value);
    return klass ? mono_class_get_name(klass) : "<unknown>";
}

std::string monoToString(MonoString* text) {
    if (!text) {
        return {};
    }
    char* utf8 = mono_string_to_utf8(text);
    if (!utf8) {
        return {};
    }
    std::string out(utf8);
    mono_free(utf8);
    return out;
}

std::string monoExceptionMessage(MonoObject* exc) {
    if (!exc) {
        return {};
    }
    MonoString* text = mono_object_to_string(exc, nullptr);
    return monoToString(text);
}

MonoInvokeArgs buildMonoInvokeArgs(MonoDomain* domain, const std::vector<ScriptValue>& args) {
    MonoInvokeArgs invokeArgs;
    invokeArgs.integers.reserve(args.size());
    invokeArgs.numbers.reserve(args.size());
    invokeArgs.booleans.reserve(args.size());
    invokeArgs.strings.reserve(args.size());
    invokeArgs.params.reserve(args.size());

    for (const auto& arg : args) {
        if (const auto* boolValue = std::get_if<bool>(&arg)) {
            invokeArgs.booleans.push_back(*boolValue ? 1 : 0);
            invokeArgs.params.push_back(&invokeArgs.booleans.back());
        } else if (const auto* integerValue = std::get_if<i64>(&arg)) {
            invokeArgs.integers.push_back(*integerValue);
            invokeArgs.params.push_back(&invokeArgs.integers.back());
        } else if (const auto* numberValue = std::get_if<f64>(&arg)) {
            invokeArgs.numbers.push_back(*numberValue);
            invokeArgs.params.push_back(&invokeArgs.numbers.back());
        } else if (const auto* stringValue = std::get_if<std::string>(&arg)) {
            invokeArgs.strings.push_back(mono_string_new(domain, stringValue->c_str()));
            invokeArgs.params.push_back(invokeArgs.strings.back());
        } else {
            invokeArgs.params.push_back(nullptr);
        }
    }

    return invokeArgs;
}

ScriptValue monoObjectToScriptValue(MonoObject* object) {
    if (!object) {
        return std::monostate {};
    }

    MonoClass* klass = mono_object_get_class(object);
    if (!klass) {
        return std::monostate {};
    }

    MonoType* type = mono_class_get_type(klass);
    if (!type) {
        return std::monostate {};
    }

    switch (mono_type_get_type(type)) {
        case MONO_TYPE_BOOLEAN:
            return (*reinterpret_cast<mono_bool*>(mono_object_unbox(object)) != 0);
        case MONO_TYPE_I1:
            return static_cast<i64>(*reinterpret_cast<int8_t*>(mono_object_unbox(object)));
        case MONO_TYPE_U1:
            return static_cast<i64>(*reinterpret_cast<uint8_t*>(mono_object_unbox(object)));
        case MONO_TYPE_I2:
            return static_cast<i64>(*reinterpret_cast<int16_t*>(mono_object_unbox(object)));
        case MONO_TYPE_U2:
            return static_cast<i64>(*reinterpret_cast<uint16_t*>(mono_object_unbox(object)));
        case MONO_TYPE_I4:
            return static_cast<i64>(*reinterpret_cast<int32_t*>(mono_object_unbox(object)));
        case MONO_TYPE_U4:
            return static_cast<i64>(*reinterpret_cast<uint32_t*>(mono_object_unbox(object)));
        case MONO_TYPE_I8:
            return static_cast<i64>(*reinterpret_cast<int64_t*>(mono_object_unbox(object)));
        case MONO_TYPE_U8:
            return static_cast<i64>(*reinterpret_cast<uint64_t*>(mono_object_unbox(object)));
        case MONO_TYPE_R4:
            return static_cast<f64>(*reinterpret_cast<float*>(mono_object_unbox(object)));
        case MONO_TYPE_R8:
            return static_cast<f64>(*reinterpret_cast<double*>(mono_object_unbox(object)));
        case MONO_TYPE_STRING:
            return monoToString(reinterpret_cast<MonoString*>(object));
        default:
            return std::monostate {};
    }
}

} // namespace
#endif

CSharpBridge::CSharpBridge()  = default;
CSharpBridge::~CSharpBridge() { shutdown(); }

bool CSharpBridge::init() {
#ifdef FELISS_CSHARP_ENABLED
    mono_config_parse(nullptr);
    m_domain = mono_jit_init("FelissScripting");
    if (!m_domain) {
        m_lastError = "mono_jit_init failed";
        FLS_ERROR("C#", m_lastError);
        return false;
    }
    FLS_INFO("C#", "Mono JIT initialized");
    return true;
#else
    m_lastError = "C# not compiled in — enable FELISS_CSHARP_ENABLED and link Mono";
    FLS_WARN("C#", m_lastError);
    return false;
#endif
}

void CSharpBridge::shutdown() {
#ifdef FELISS_CSHARP_ENABLED
    if (m_domain) {
        mono_jit_cleanup(static_cast<MonoDomain*>(m_domain));
        m_domain = nullptr;
        FLS_INFO("C#", "Mono shutdown");
    }
#endif
}

void CSharpBridge::update(f32) {}

bool CSharpBridge::loadFile(const std::string& path) {
    return loadAssembly(path);
}

bool CSharpBridge::loadString(const std::string&, const std::string&) {
    FLS_WARN("C#", "Runtime string compilation not supported — compile to .dll first");
    return false;
}

bool CSharpBridge::loadAssembly(const std::string& dllPath) {
#ifdef FELISS_CSHARP_ENABLED
    if (!m_domain) return false;
    MonoAssembly* asm_ = mono_domain_assembly_open(
        static_cast<MonoDomain*>(m_domain), dllPath.c_str());
    if (!asm_) {
        m_lastError = "Failed to open assembly: " + dllPath;
        FLS_ERROR("C#", m_lastError);
        return false;
    }
    m_assembly = asm_;
    m_image    = mono_assembly_get_image(asm_);
    if (std::find(m_loadedAssemblies.begin(), m_loadedAssemblies.end(), dllPath) == m_loadedAssemblies.end()) {
        m_loadedAssemblies.push_back(dllPath);
    }
    FLS_INFOF("C#", "Loaded assembly: " << dllPath);
    return true;
#else
    FLS_WARN("C#", "loadAssembly: Mono not enabled");
    return false;
#endif
}

void* CSharpBridge::createInstance(const std::string& className) {
#ifdef FELISS_CSHARP_ENABLED
    if (!m_image) return nullptr;
    std::string ns, cls;
    auto dot = className.rfind('.');
    if (dot != std::string::npos) { ns = className.substr(0, dot); cls = className.substr(dot+1); }
    else { cls = className; }
    MonoClass* klass = mono_class_from_name(static_cast<MonoImage*>(m_image), ns.c_str(), cls.c_str());
    if (!klass) { m_lastError = "Class not found: " + className; return nullptr; }
    MonoObject* obj = mono_object_new(static_cast<MonoDomain*>(m_domain), klass);
    if (!obj) return nullptr;
    mono_runtime_object_init(obj);
    uint32_t h = mono_gchandle_new(obj, false);
    return reinterpret_cast<void*>(static_cast<uintptr_t>(h));
#else
    return nullptr;
#endif
}

void CSharpBridge::destroyInstance(void* handle) {
#ifdef FELISS_CSHARP_ENABLED
    if (!handle) return;
    mono_gchandle_free(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle)));
#endif
}

bool CSharpBridge::callMethod(void* handle, const std::string& method,
                               const std::vector<ScriptValue>& args, ScriptValue* out) {
#ifdef FELISS_CSHARP_ENABLED
    if (!handle || !m_image) return false;
    MonoObject* obj = mono_gchandle_get_target(
        static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handle)));
    if (!obj) return false;
    MonoClass*  klass  = mono_object_get_class(obj);
    MonoMethod* meth   = mono_class_get_method_from_name(klass, method.c_str(),
                                                          (int)args.size());
    if (!meth) {
        m_lastError = "Method not found: " + method;
        return false;
    }
    MonoInvokeArgs invokeArgs = buildMonoInvokeArgs(static_cast<MonoDomain*>(m_domain), args);
    MonoObject* exc = nullptr;
    MonoObject* result = mono_runtime_invoke(
        meth,
        obj,
        invokeArgs.params.empty() ? nullptr : invokeArgs.params.data(),
        &exc);
    if (exc) {
        m_lastError = monoExceptionMessage(exc);
        if (m_lastError.empty()) {
            m_lastError = "Exception in " + method;
        }
        FLS_ERRORF("C#", method << " threw: " << m_lastError);
        return false;
    }
    if (out) {
        *out = monoObjectToScriptValue(result);
    }
    return true;
#else
    return false;
#endif
}

bool CSharpBridge::callGlobal(const std::string& fn,
                               const std::vector<ScriptValue>&, ScriptValue*) {
    FLS_WARNF("C#", "callGlobal not supported: " << fn);
    return false;
}

bool CSharpBridge::setProp(void*, const std::string&, const ScriptValue&) { return false; }
ScriptValue CSharpBridge::getProp(void*, const std::string&) { return std::monostate{}; }

void CSharpBridge::bindAPI(Engine& engine) {
    m_engine = &engine;
    FLS_INFO("C#", "Engine API bound (internal calls registration required)");
}

void CSharpBridge::reload() {
    auto files = m_loadedAssemblies;
    for (auto& f : files) {
        loadAssembly(f);
    }
}

} // namespace Feliss
