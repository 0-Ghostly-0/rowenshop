# Rowen Fragment — build configuration.
# Same layout and pipeline as Rowen Vocal Polish. See docs/BUILD_WINDOWS.md.

cmake_minimum_required(VERSION 3.22)

project(RowenFragment
    VERSION 0.5.0
    DESCRIPTION "Rowen Fragment - creative audio fragmentation"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ROWEN_BUILD_TESTS "Build the standalone core test suite" ON)
option(ROWEN_DSP_TESTS_ONLY "Build only the core tests (needs no JUCE or network)" OFF)

if(NOT ROWEN_DSP_TESTS_ONLY)

# --- JUCE ---------------------------------------------------------------------
set(ROWEN_JUCE_VERSION 8.0.4)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/JUCE/CMakeLists.txt")
    message(STATUS "Rowen: using JUCE from libs/JUCE")
    add_subdirectory(libs/JUCE)
else()
    message(STATUS "Rowen: libs/JUCE not found, fetching JUCE ${ROWEN_JUCE_VERSION}")
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        ${ROWEN_JUCE_VERSION}
        GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# --- Plugin target ------------------------------------------------------------
juce_add_plugin(RowenFragment
    PRODUCT_NAME                "Rowen Fragment"
    COMPANY_NAME                "Rowen"
    COMPANY_WEBSITE             "https://rowen.work"
    BUNDLE_ID                   com.rowen.fragment
    PLUGIN_MANUFACTURER_CODE    Rown
    PLUGIN_CODE                 Rfr1
    FORMATS                     VST3
    VST3_CATEGORIES             Fx Delay Pitch Shift
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD     FALSE)

target_sources(RowenFragment PRIVATE
    src/core/PluginProcessor.cpp
    src/core/PluginEditor.cpp
    src/dsp/FragmentCore.cpp
    src/params/ParameterLayout.cpp
    src/presets/FactoryPresets.cpp
    src/presets/PresetManager.cpp
    src/ui/components/FragmentTopBar.cpp
    src/ui/theme/RowenLookAndFeel.cpp
    src/ui/theme/RowenTypography.cpp)

target_include_directories(RowenFragment PRIVATE src)

# --- Embedded brand assets ------------------------------------------------------
# Same brand assets as Rowen Vocal Polish (logo, floral texture, Space Grotesk /
# Poppins / JetBrains Mono — all SIL OFL, free for commercial embedding). Font
# files are picked up automatically from assets/fonts/; if a file is absent the
# UI falls back to the system sans-serif and still builds.
file(GLOB ROWEN_FONT_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/*.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/*.otf")

juce_add_binary_data(RowenFragmentAssets
    NAMESPACE BinaryData
    SOURCES
        assets/brand/logo-icon.png
        assets/brand/logo-wordmark.png
        assets/brand/bg-floral.jpeg
        ${ROWEN_FONT_FILES})

target_link_libraries(RowenFragment PRIVATE RowenFragmentAssets)

target_compile_definitions(RowenFragment PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=1)   # required until a commercial JUCE license is active

target_link_libraries(RowenFragment
    PRIVATE
        juce::juce_audio_utils
        juce::juce_data_structures   # juce::UndoManager for the preset/randomize undo stack
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

endif() # NOT ROWEN_DSP_TESTS_ONLY

# --- Standalone core tests (no JUCE dependency) -------------------------------
if(ROWEN_BUILD_TESTS)
    enable_testing()
    add_executable(RowenFragmentTests tests/CoreTests.cpp src/dsp/FragmentCore.cpp)
    target_include_directories(RowenFragmentTests PRIVATE src)
    target_compile_features(RowenFragmentTests PRIVATE cxx_std_20)
    find_package(Threads REQUIRED)
    target_link_libraries(RowenFragmentTests PRIVATE Threads::Threads)
    add_test(NAME core COMMAND RowenFragmentTests)
endif()
