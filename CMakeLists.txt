cmake_minimum_required(VERSION 3.22)
project(SergioModular VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(APPLE)
  # universal by default (release installers); -DPF_ARCHS=arm64 for a faster native-only dev build
  set(PF_ARCHS "arm64;x86_64" CACHE STRING "macOS architectures to build")
  set(CMAKE_OSX_ARCHITECTURES "${PF_ARCHS}" CACHE STRING "" FORCE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "" FORCE)
  add_compile_definitions(JUCE_SILENCE_XCODE_15_LINKER_WARNING=1)
  add_link_options("-Wl,-ld_classic")
endif()

# ---- JUCE: use a local checkout when present, otherwise fetch 8.0.6 --------
if(EXISTS "$ENV{HOME}/JUCE/CMakeLists.txt")
  message(STATUS "Using local JUCE at $ENV{HOME}/JUCE")
  add_subdirectory("$ENV{HOME}/JUCE" JUCE)
else()
  include(FetchContent)
  FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()

set(PF_FORMATS VST3 Standalone)
if(APPLE)
  list(APPEND PF_FORMATS AU)
endif()

set(PF_SOURCES
  src/plugin/PluginProcessor.cpp)

juce_add_binary_data(SergioAssets SOURCES
  assets/calabi_icon.png
  assets/calabi_wordmark.png)

function(pf50_common target)
  target_compile_definitions(${target} PUBLIC
    JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
  if(APPLE)
    target_link_options(${target} PRIVATE "-Wl,-ld_classic")
  endif()
  target_link_libraries(${target} PRIVATE
    SergioAssets
    juce::juce_audio_utils
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
endfunction()

# ---- Instrument -------------------------------------------------------------
juce_add_plugin(Paperface50
  PRODUCT_NAME "SERGIO"
  COMPANY_NAME "Calabi Yau"
  BUNDLE_ID com.calabiyau.sergio
  PLUGIN_MANUFACTURER_CODE Clby
  PLUGIN_CODE Srg0
  FORMATS ${PF_FORMATS}
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(Paperface50 PRIVATE ${PF_SOURCES})
target_compile_definitions(Paperface50 PRIVATE PF50_FX=0)
pf50_common(Paperface50)

# ---- FX variant -------------------------------------------------------------
juce_add_plugin(Paperface50FX
  PRODUCT_NAME "SERGIO FX"
  COMPANY_NAME "Calabi Yau"
  BUNDLE_ID com.calabiyau.sergiofx
  PLUGIN_MANUFACTURER_CODE Clby
  PLUGIN_CODE SrgF
  FORMATS ${PF_FORMATS}
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(Paperface50FX PRIVATE ${PF_SOURCES})
target_compile_definitions(Paperface50FX PRIVATE PF50_FX=1)
pf50_common(Paperface50FX)

# ---- Gauntlet DSP suite (no JUCE) ------------------------------------------
add_executable(pf50_gauntlet tests/GauntletTests.cpp)
set_target_properties(pf50_gauntlet PROPERTIES CXX_STANDARD 17)

# ---- UI snapshot tool -------------------------------------------------------
juce_add_console_app(pf50_snapshot PRODUCT_NAME "pf50_snapshot")
target_sources(pf50_snapshot PRIVATE tools/Snapshot.cpp src/plugin/PluginProcessor.cpp)
target_compile_definitions(pf50_snapshot PRIVATE PF50_FX=0 JUCE_MODAL_LOOPS_PERMITTED=1)
pf50_common(pf50_snapshot)
target_link_libraries(pf50_snapshot PRIVATE juce::juce_gui_extra)

enable_testing()
add_test(NAME gauntlet COMMAND pf50_gauntlet)
