cmake_minimum_required(VERSION 3.10)

project(chessx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_11)
target_compile_definitions(project_options INTERFACE
  QT_DEPRECATED_WARNINGS
  QT_NO_CAST_TO_ASCII
  QT_USE_QSTRINGBUILDER
)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13)

option(ENABLE_SOUNDS "Enable sounds (requires Qt5::Multimedia)" ON)
option(ENABLE_TTS "Enable text-to-speech (requires Qt5::TextToSpeech)" ON)
option(ENABLE_SCID_SUPPORT "Enable support for Scid database format (*.si4)" ON)

add_subdirectory(dep)

set(REQUIRED_QT_COMPONENTS
  Core Xml DBus Network OpenGL Svg PrintSupport Gui Widgets
)

if (ENABLE_SOUNDS)
  list(APPEND REQUIRED_QT_COMPONENTS Multimedia MultimediaWidgets)
endif()

if (ENABLE_TTS)
  list(APPEND REQUIRED_QT_COMPONENTS TextToSpeech)
endif()

find_package(Qt5 REQUIRED
  LinguistTools
  "${REQUIRED_QT_COMPONENTS}"
  Test
) 

# put translations.rc alongside generated .qm files
configure_file(translations.qrc ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

# list of .ts files to compile into .qm
set(TRANSLATIONS_SRC_FILES
  # i18n/chessx_da.ts
  i18n/chessx_de.ts
  # i18n/chessx_fr.ts
  # i18n/chessx_it.ts
  # i18n/chessx_cz.ts
  # i18n/chessx_ru.ts
)

set_source_files_properties(${TRANSLATIONS_SRC_FILES}
  PROPERTIES
  OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/i18n"
)

qt5_add_translation(TRANSLATIONS_BIN_FILES ${TRANSLATIONS_SRC_FILES})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_subdirectory(src)

set(OTHER_FILES
  #[[
  data/templates/pgn-default.template
  data/templates/notation-default.template
  data/templates/latex-default.template
  data/templates/html-default.template
  ChangeLog
  COPYING
  ChangeLog.txt
  data/help/about.css
  data/help/about-dark.css
  data/help/about0.html
  data/help/about1.html
  data/help/about1a.html
  data/help/about2.html
  data/help/about3.html
  data/help/about4.html
  data/help/about5.html
  data/help/about6.html
  setup7-64.iss
  setup7-32.iss
  setupXP.iss
  data/styles/orange.css
  unix/chessx.desktop
  ]]
)

add_executable(chessx WIN32
  src/gui/main.cpp
  resources.qrc
  ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
  ${OTHER_FILES}
)

target_link_libraries(chessx PRIVATE
  project_options
  quazip
  bitboard
  board
  eco
  gui
)

if (CMAKE_HOST_APPLE)
  # Make macOS bundle instead of bare executable
  set_target_properties(chessx PROPERTIES
    MACOSX_BUNDLE TRUE
    # TODO: use variables in Info.plist template
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/mac_osx/Info.plist"
  )

  # embed required resources
  target_sources(chessx PRIVATE
    mac_osx/Info.plist
    mac_osx/qt_menu.nib
    data/images/chessx.icns
  )

  set_property(
    TARGET chessx
    APPEND PROPERTY
    RESOURCE
      mac_osx/qt_menu.nib
      data/images/chessx.icns
  )

  # TODO: embed engines & timeseal

endif()

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)

option(ENABLE_TESTING "Enable testing" ON)
if (ENABLE_TESTING)
  enable_testing()
  add_subdirectory(tests/unittests)
endif()

