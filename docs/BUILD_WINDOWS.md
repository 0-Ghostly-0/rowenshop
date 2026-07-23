# Building Rowen Fragment on Windows

Written for limited C++ experience. Follow top to bottom; each step tells you
how to check it worked.

## 1. Install the tools (one time)

1. **Visual Studio 2022 Community** — free from visualstudio.microsoft.com.
   During install, tick the workload **"Desktop development with C++"**.
   (This includes the C++ compiler and Windows SDK.)
2. **CMake** — from cmake.org/download (Windows x64 Installer).
   During install, choose **"Add CMake to the system PATH"**.
3. **Git** — from git-scm.com (defaults are fine).

Check: open a new "Command Prompt" and run `cmake --version` and
`git --version`. Both should print version numbers.

## 2. Get the source and JUCE

```bat
cd %USERPROFILE%\Documents
git clone <your-repo-url> RowenFragment
cd RowenFragment
```

JUCE (the plugin framework) can be fetched automatically, but pinning it as a
submodule is more reliable:

```bat
git clone --depth 1 --branch 8.0.4 https://github.com/juce-framework/JUCE.git libs\JUCE
```

If you skip this, CMake downloads the same version by itself in the next step
(needs internet once).

## 3. Configure

```bat
cmake -B build -G "Visual Studio 17 2022" -A x64
```

Expected: several lines ending in `-- Generating done` with no `Error`.
This can take a few minutes the first time.

## 4. Build

```bat
cmake --build build --config Release
```

Expected: ends without `error` lines. The plugin is created at:

```
build\RowenFragment_artefacts\Release\VST3\Rowen Fragment.vst3
```

## 5. Run the automated tests

```bat
ctest --test-dir build -C Release --output-on-failure
```

Expected: `100% tests passed`.

## 6. Install the plugin for your DAW

Copy the whole `Rowen Fragment.vst3` folder to:

```
C:\Program Files\Common Files\VST3\
```

(You'll need to approve the admin prompt.) Then rescan plugins in your DAW
(FL Studio: Options → Manage plugins → Find plugins; Ableton: Options →
Plug-Ins, toggle VST3 off/on; Reaper: Options → Preferences → VST → Re-scan.)

## 7. Validate (strongly recommended before any DAW testing)

Download **pluginval** from github.com/Tracktion/pluginval/releases
(pluginval_Windows.zip), then:

```bat
pluginval.exe --strictness-level 8 --validate "C:\Program Files\Common Files\VST3\Rowen Fragment.vst3"
```

Expected: `ALL TESTS PASSED`.

## If the build fails

- **`cmake` is not recognized** — reopen the Command Prompt (PATH updates only
  apply to new windows), or reinstall CMake with the PATH option ticked.
- **`Could not find any instance of Visual Studio`** — the C++ workload isn't
  installed; rerun the Visual Studio Installer and add "Desktop development
  with C++".
- **Errors mentioning `juce::` or JUCE headers** — the JUCE download failed or
  is the wrong version. Delete the `build` folder, do the `git clone ... libs\JUCE`
  step above, then configure again.
- **Anything else** — copy the FIRST error line (later errors are usually
  echoes of the first) and send it to me; first errors are almost always
  one-line fixes.

## Debug builds

Use `--config Debug` in step 4 for a debuggable build. Debug builds are slower
and not for listening tests.
