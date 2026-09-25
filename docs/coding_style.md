# C++ coding style

This guide applies to repository-owned C++ headers and implementations throughout `src/`, excluding `src/Dependencies/`. It is intended for human developers and AI coding tools. It combines the conventions shared across the source tree with the maintainer's requirements for future code.

Follow these rules when adding or changing code. Existing files contain historical exceptions, including unbraced branches, comma-separated declarations, inconsistent names, and lambdas. Those exceptions do not override this guide. Preserve existing API names, SDK interfaces, protocol layouts, and configuration keys unless the task explicitly requires changing them. Keep unrelated formatting changes out of a functional change.

## Naming

- Prefix member variables with `m_`, followed by a lowercase name: `m_name`, `m_frameRate`, `m_pVisionBase`. This also applies to members of repository-owned structs.
- Local variables and parameters do **not** have the `m_` prefix: `pVisionBase`, `vInputs`, `frameRate`.
- Use lower camel case for variable and function names: `readFrame`, `setTargetFPS`, `getName`, `bRunning`. Start lowercase and capitalize subsequent words. Preserve established domain acronyms and SDK spelling where appropriate.
- Short type or role hints are recommended when they make a name clearer; they are not compulsory. Do not build a complicated prefix system or infer ownership from a prefix.

| Hint | Common meaning | Member example | Local example |
| --- | --- | --- | --- |
| `p` | Pointer or pointer-backed buffer | `m_pVisionBase` | `pVisionBase` |
| `v` | Container or mathematical vector | `m_vInputs` | `vInputs` |
| `b` | Boolean state or option | `m_bLog` | `bReady` |
| `n` | Count or size | `m_nFrames` | `nFrames` |
| `i` | Index | `m_iFrame` | `iFrame` |
| `t` | Time or timestamp | `m_tStamp` | `tNow` |

Use familiar abbreviations to keep names short: `cfg`, `ctrl`, `msg`, `buf`, `pos`, `dim`, `src`, `dst`, and `idx` are useful in an appropriate context. Retain enough meaning to distinguish nearby variables. Single-letter names such as `i`, `j`, or `x` suit short loops and mathematical expressions; longer-lived state needs more context. Include units where ambiguous, as in `m_tIMUpairToleranceNs`.

Classes that run their own threads use a leading underscore, such as `_VisionBase`, `_Camera`, or `_Thread`. The existing module inheritance families also retain that prefix, including base classes whose worker behavior is supplied by a subclass. Ordinary helper classes use names such as `ModuleMgr`, `JsonCfg`, and `SharedMem`. Match the existing family when extending it; legacy exceptions are not a reason to rename public classes during unrelated work.

Type names have several established families: class-style names such as `ScCtrl`, and uppercase struct/enum names such as `IMU_DATA`, `ACTUATOR_V`, and `THREAD_STATE`. Enum values also follow their local family, such as `thread_run` and `actuator_ready`. Match nearby related types rather than renaming an established family. The lower camel case rule concerns variables and functions, not these existing type or constant names.

## Braces, indentation, and spaces

Use braces on separate lines (Allman style) for namespaces, classes, structs, functions, and control-flow blocks. **Every `if`, `else`, `for`, `while`, and `do` body must have braces, even when it contains only one statement.** An `else if` chain is fine; each branch body must be braced. Never use an unbraced early return.

```cpp
if (bReady)
{
	readFrame();
}
else if (bRetry)
{
	reopen();
}
else
{
	return false;
}
```

Tabs are the dominant indentation in the repository; use one tab per nesting level in new files. Some existing files consistently use four spaces; preserve their indentation when editing them. Do not mix indentation styles within a block. Indent the contents of `namespace kai`, classes, functions, and control-flow blocks. Class access labels align with the class's opening brace; members are indented one level further.

Use a space after control keywords, around binary and assignment operators, and after commas. Do not add a space between a function name and `(` or inside ordinary parentheses. Prefer pointer/reference spacing such as `_VisionBase *pVisionBase` and `const json &j`; retain consistent local spacing in existing declarations.

```cpp
int nFrames = 0;
float scale = width / inputWidth;
_VisionBase *pVisionBase = nullptr;

for (int i = 0; i < nFrames; i++)
{
	processFrame(i, scale);
}
```

Wrap long signatures, calls, and conditions at logical boundaries and align continuation lines consistently. The repository has no checked-in formatter configuration or uniform hard line-length limit; prioritize readable expressions. Avoid reflowing an entire file to apply a different formatter default.

## Paragraphs and logical groups

Use a single blank line to separate meaningful sections. Keep closely related lines together: declarations of related values or the same type, settings for one device, one configuration group, one calculation, or one complete loop or conditional. Prefer grouping by purpose when grouping only by type would separate related state.

Separate function definitions and adjacent class, struct, and enum definitions with blank lines. Separate fields from member functions in a struct. Within functions, separate validation, setup, processing, and result handling when these are distinct steps. Do not insert a blank line between every statement or split an `if` from its `else`.

```cpp
struct FrameCtrl
{
	bool m_bAutoExposure = true;
	int m_tExposureUs = 4000;

	int m_width = 640;
	int m_height = 480;

	void resetSize(void)
	{
		m_width = 640;
		m_height = 480;
	}
};

struct FrameStats
{
	int m_nFrames = 0;
	uint64_t m_tLastFrameNs = 0;
};
```

Use a short section comment when a group's purpose is not already clear. A blank line often provides enough separation without a comment.

## Declarations and statements

Declare **one variable per declaration and one statement per line**. This applies to locals, members, pointers, and struct fields. Do not compress assignments, calls, returns, or branch bodies onto one line.

```cpp
int a = 0;
int b = 1;
int c = 2;

_VisionBase *pInput = nullptr;
_VisionBase *pOutput = nullptr;
```

Do not write `int a = 0, b = 1, c = 2;`, `a = 0; b = 1;`, or `if (bReady) return true;`. The three clauses of a normal `for` header, function parameter lists, and aggregate initializers are not multiple standalone statements; do not split them mechanically. Still avoid declaring several loop variables in one declaration.

Initialize values deliberately. Simple member defaults commonly belong in the header next to the declaration; constructor setup handles resources and values that require runtime work. Use `nullptr` for new null pointers. Keep variables near their use, with related declarations grouped together. Use `const` for values that do not change and references for inputs where copying is unnecessary. Use `auto` when the type is evident or spelling out an iterator would obscure the operation; keep explicit types where units, size, or an API contract matter.

## Flat control flow

Validate preconditions first and return early on failure. In loops, use an early `continue` to skip an invalid item. Keep the normal execution path at the lowest practical indentation level. Do not add an `else` after a branch that has already returned, continued, or otherwise exited.

```cpp
bool _FrameReader::readFrame(void)
{
	if (m_pVisionBase == nullptr)
	{
		return false;
	}

	if (!m_pVisionBase->check())
	{
		return false;
	}

	if (!captureFrame())
	{
		return false;
	}

	publishFrame();
	return true;
}
```

This is an illustrative method, not a class provided by the repository. The same pattern applies to `void` functions with `return;` and pointer-returning functions with `return nullptr;`.

Keep functions focused on one operation or lifecycle stage. Extract a clearly named helper when it removes substantial nesting or isolates a reusable operation. Avoid deeply nested conditionals, nested ternary expressions, and expressions that combine several side effects. Necessary loops and branches are fine; do not obscure their purpose just to reduce a nesting count. Ensure early exits still release resources and locks.

## Functions, classes, and files

- Place repository module code in `namespace kai`. Follow an existing helper namespace when extending that helper. Use file-local helpers for implementation details that do not belong in a public header.
- Pair a class's `.h` and `.cpp` using the existing class spelling, such as `_Camera.h` and `_Camera.cpp`. Header-only templates and small utilities are established exceptions.
- Use the path-based include guards found in adjacent headers, for example `OpenKAI_src_Vision__Camera_H_`. Include a `.cpp` file's own header first, then its other dependencies. Keep local quoted includes and system/SDK angle-bracket includes in understandable groups.
- Preserve the subsystem's `WITH_*` and dependency's `USE_*` feature guards. Optional SDK types and includes must remain behind the appropriate build conditions.
- Group class declarations by role: constructor/destructor, lifecycle methods, public operations, implementation helpers and callbacks, then member state. Preserve the base class interface and existing access boundaries; do not expose a member merely to follow a layout example.
- Keep `.cpp` definitions in a corresponding readable order. Templates and small accessors may remain in headers; substantial processing belongs in an implementation file.
- Match the surrounding use of `(void)` or `()` for empty parameter lists. Both occur in this C++ repository. Mark new overriding methods with `override` where appropriate; existing declarations often spell out `virtual`.

Many modules use this lifecycle vocabulary:

| Method | Typical responsibility |
| --- | --- |
| `init` | Initialize the base and read local configuration/defaults |
| `link` | Resolve references to other modules through `ModuleMgr` |
| `start` | Start the module's worker thread(s) |
| `check` | Validate prerequisites needed for an operation |
| `update` | Run the worker loop and call named processing methods |
| `stop`, `close`, destructor | Stop work and release owned resources as required by the class |
| `console`, `draw` | Report state or render diagnostics |

Reuse these names and the relevant base implementation when extending a module. Keep thread entry adapters short: the established `getUpdate` static function delegates to the instance's `update`. Preserve the module's pause/resume, pacing, and shutdown behavior. Distinguish owned resources from linked/borrowed module pointers, and document lifetime or locking constraints where they are not obvious.

## Exceptions, macros, and callbacks

**Avoid `try`/`catch` unless it is necessary or a third-party SDK/API requires exception handling.** Use the existing return-value/status conventions for ordinary validation and recoverable failures. Necessary catches should surround the smallest useful boundary, handle specific errors, and provide meaningful failure information. JSON parsing in `JsonCfg` is an example of such a boundary. Do not use exceptions for routine branching or add a catch-all that silently reports success.

**Avoid unnecessary macros.** Introduce a macro only when it significantly improves readability or preprocessing is required. Prefer an ordinary named function, template, or typed `const`/`constexpr` value for new logic and constants. Include guards, feature switches, logging, and existing module registration macros are established uses.

The codebase already uses return and cleanup macros from [macro.h](../src/Base/macro.h):

| Existing macro | Behavior |
| --- | --- |
| `IF_(condition)` / `IF_F(condition)` / `IF_N(condition)` | Return void / `false` / `nullptr` when the condition is true |
| `NULL_(p)` / `NULL_F(p)` / `NULL_N(p)` | Return when the pointer is null |
| `IF_CONT(condition)` | Continue the enclosing loop when the condition is true |
| `IF_Le_F(condition, message)` | Log an error and return `false` when the condition is true |
| `DEL(p)` / `DEL_ARRAY(p)` | Delete the object/array and set the pointer to `nullptr` |

Recognize these when reading existing code, and reuse them only where their meaning is clear and they improve readability. Prefer explicit braced guards for new control flow when a macro would hide the exit or its return value. Do not create new macro families to shorten a few lines. Some existing macros expand to control-flow statements, so always brace an enclosing branch or loop and avoid arguments with side effects.

**Avoid lambda function definitions unless they are unavoidable.** Prefer a member function, a file-local helper, a static callback adapter, or a named callable when state is needed. A callback-taking SDK does not by itself require a lambda. If an unavoidable API constraint calls for one, keep the lambda small, delegate substantive work to a named function, and make captured state and its lifetime explicit. Keep the execution flow easy to follow.

## Configuration, logging, and comments

Group `jKv` configuration reads by the same device or concern as the corresponding member declarations. Retain defaults for optional values and validate required ones. Resolve inter-module references in `link` using the existing manager conventions. Configuration names often mirror member names without `m_`, but existing JSON keys are contracts: do not rename them automatically to match a style rule.

Use the existing logging helpers where appropriate for the surrounding class. Include enough context to identify the failed operation or device. Do not add repeated logs inside a hot loop without a clear diagnostic purpose.

Write comments that explain intent, units, coordinate systems, SDK restrictions, ownership, locking, or a non-obvious decision. Prefer `//` for brief notes and section labels, with a space after `//`. Avoid narrating obvious assignments. Preserve existing license and attribution text; historical file banners do not require invented author or creation-date entries in new files.

## Developer and AI review checklist

- Member names use `m_`; local names do not. Variable and function names start lowercase, with clear camel case and useful abbreviations.
- Thread-owning classes follow the leading-underscore module convention.
- Related declarations and operations are grouped, with blank lines between logical sections and type definitions.
- Every control-flow body is braced, including single-statement guards and loops.
- Each declaration introduces one variable; each line contains one standalone statement.
- Guard returns and loop skips keep the normal path flat; helpers have clear responsibilities.
- Exceptions, new macros, and lambdas are used only under the limited conditions described above.
- Existing interfaces, configuration keys, feature guards, resource ownership, and thread behavior remain correct.
- Changes follow this guide without copying historical exceptions or reformatting unrelated code.

## Basis in the source tree

The survey covered C++ files in every source folder and its subfolders, including `src/main.cpp`, with `src/Dependencies/` excluded. The references below are examples of observed conventions, not files that satisfy every future-code requirement.

| Areas reviewed | Representative references and patterns |
| --- | --- |
| `Base`, `Module`, `main.cpp` | [BASE.h](../src/Base/BASE.h), [_ModuleBase.cpp](../src/Base/_ModuleBase.cpp), [ModuleMgr.cpp](../src/Module/ModuleMgr.cpp): lifecycle, guard returns, configuration, class layout |
| `Actuator`, including `Motor` and `Articulated` | [_ActuatorBase.h](../src/Actuator/_ActuatorBase.h): grouped state, struct members, short domain names |
| `Autopilot`, including `ArduPilot` and `Drive` | [_APmav_base.h](../src/Autopilot/FC/ArduPilot/_APmav_base.h), [_Drive.cpp](../src/Autopilot/Drive/_Drive.cpp): module families, configuration, worker methods |
| `IO`, `Protocol`, `IPC` | [_IObase.h](../src/IO/_IObase.h), [_ProtocolBase.cpp](../src/Protocol/_ProtocolBase.cpp), [SharedMem.h](../src/IPC/SharedMem.h): buffers, callbacks, resource helpers |
| `Sensor`, including `Distance` and `LiDAR`; `Navigation` | [_IMUbase.h](../src/Sensor/_IMUbase.h), [_GPS.cpp](../src/Navigation/_GPS.cpp): timestamps, units, grouped device state |
| `Swarm`, `ROS` | [_SwarmBase.h](../src/Swarm/_SwarmBase.h), [_ROS_fastLio.h](../src/ROS/_ROS_fastLio.h): thread modules and external API adapters |
| `Vision`, including `RGBD` and `Pipeline`; `Detector`, `Tracker` | [_VisionBase.h](../src/Vision/_VisionBase.h), [_Scepter.h](../src/Vision/RGBD/_Scepter.h), [_DetectorBase.cpp](../src/Detector/_DetectorBase.cpp), [_TrackerBase.h](../src/Tracker/_TrackerBase.h): optional name hints, device options, module inheritance |
| `SLAM`, `Universe`, including `Grid`, `Geometry`, and point-cloud pipelines/registration | [_SLAMbase.h](../src/SLAM/_SLAMbase.h), [_GeometryBase.h](../src/Universe/Geometry/_GeometryBase.h), [_PCtransform.cpp](../src/Universe/Geometry/PointCloud/Pipeline/_PCtransform.cpp): geometry state, feature guards, formatting variations |
| `Canvas`, `Tools`, `UI`, including `Viewer`, `Web`, and `ImGUI` | [_Object.h](../src/Canvas/_Object.h), [_CamCalib.cpp](../src/Tools/_CamCalib.cpp), [_UIbase.h](../src/UI/_UIbase.h), [WebGLIMProtocol.h](../src/UI/Viewer/Web/WebGLIMProtocol.h): grouped methods, comments, specialized helper namespaces |
| `Arithmetic`, `Control`, `Filter`, `Primitive` | [Destimator.h](../src/Arithmetic/Destimator.h), [PID.h](../src/Control/PID.h), [Average.h](../src/Filter/Average.h), [tSwap.h](../src/Primitive/tSwap.h): mathematical abbreviations and header-only helpers |
| `Compute`, `Data`, `Net` | [clBase.h](../src/Compute/OpenCL/clBase.h), [_FileBase.cpp](../src/Data/_FileBase.cpp), [HttpServer.cpp](../src/Net/HttpServer.cpp): SDK wrappers, file operations, existing naming/callback exceptions |
| `Solver`, `State`, `Utility` | [_SATbase.h](../src/Solver/_SATbase.h), [StateBase.h](../src/State/StateBase.h), [utilTime.h](../src/Utility/utilTime.h): domain structs, helper functions, constants |
