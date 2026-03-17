# Feliss
Feliss is a C++ game engine codebase with a shared engine core, a dedicated editor, and a separate runtime executable.

Current architecture:

- `EngineCore` : shared engine library used by both editor and runtime
- `FelissEditor` : editor application, output `feliss.exe`
- `FelissRuntime` : lean runtime application, output `feliss_runtime.exe`
- `feliss_core` : C API bridge for integrations and extensions

## Status

The project is in active development. The engine already has a basic editor shell, scene/world model, rendering path, extension loading, and project configuration flow, but several systems are still foundation-level rather than production-complete.

Implemented or partially implemented:

- shared engine core
- editor/runtime split
- OpenGL renderer path
- scene/entity/component world
- extension loading through `feliss_api.h`
- project configuration via `ProjectConfig.yaml`
- editor-only Python tooling hooks
- asset registry foundation with `.meta` generation
- physics backend abstraction foundation

Not fully complete yet:

- full PBR pipeline
- full FBX import pipeline
- production Vulkan / DirectX 12 renderer
- reflection-driven serialization for all systems
- full Play-In-Editor workflow
- full undo/redo command system

## Project Layout

```text
FelissEngine/
├─ apps/             # Editor and runtime entry points
├─ assets/           # Project assets
├─ extensions/       # Native extensions/plugins
├─ include/          # Public headers
├─ src/              # Engine/editor/runtime source
├─ tests/            # Tests
├─ tools/            # Tooling scripts, including Python helpers
├─ ProjectConfig.yaml
├─ CMakeLists.txt
└─ requirements.txt
```

Important source areas:

- `src/core/` : engine boot, logging, filesystem, timers, task scheduler
- `src/ecs/` : scene/entity/component world model
- `src/renderer/` : render pipeline and backends
- `src/editor/` : editor UI, panels, session, exporters
- `src/runtime/` : runtime application entry flow
- `src/assets/` : asset registry and metadata handling
- `src/physics/` : backend abstraction for physics
- `src/project/` : project configuration loading/saving
- `tools/python/` : editor automation and asset tooling scripts

## Build

### Configure

```powershell
cd C:\Users\BEST\Desktop\FelissEngine
cmake -S . -B build-native -G Ninja
```

### Build everything relevant

```powershell
cmake --build build-native --target EngineCore FelissEditor FelissRuntime feliss_core
```

### Build editor only

```powershell
cmake --build build-native --target FelissEditor
```

### Build runtime only

```powershell
cmake --build build-native --target FelissRuntime
```

## Output Binaries

Typical outputs:

- `build-native\feliss.exe`
- `build-native\feliss_runtime.exe`
- `build-native\EngineCore.dll`
- `build-native\feliss_core.dll`

## Python Tooling

Feliss includes editor-only Python tooling under `tools/python/`.

Examples:

- `import_texture.py`
- `build_materials.py`
- `fbx_postprocess.py`
- `generate_lods.py`
- `pack_atlas.py`
- `header_tool.py`

Current Python tooling uses only the standard library, so `requirements.txt` does not list any mandatory third-party packages yet.

Install step:

```powershell
pip install -r requirements.txt
```

## Project Configuration

Project-wide settings live in `ProjectConfig.yaml`.

Current config areas include:

- default script layer: `CSharp` or `Lua`
- script toggles
- preferred render API
- physics backend
- default scene
- asset root

Editor and runtime both read this file so they stay aligned on shared project behavior.

## Extensions and API

The public C API is exposed through:

- `include/feliss/feliss_api.h`

This API can interact with native extensions in `extensions/` through extension management functions such as load, unload, enable, disable, and query operations.

## Dependencies

Main dependencies currently used by the project:

- CMake
- Ninja
- C++20 compiler
- GLFW
- OpenGL
- ImGui

Optional or in-progress dependencies:

- Lua
- Vulkan SDK
- DirectX 12 toolchain
- Python for tooling
- external physics SDKs such as AsterCore, PhysX, or Havok

## Notes

- The editor is intended to be the main content creation program.
- The runtime is intended to stay lean and suitable for packaged/shipped games.
- The engine is being refactored toward a more complete shared-data, metadata-driven architecture.

## License

See `LICENSE.txt`.
