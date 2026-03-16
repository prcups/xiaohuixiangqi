cmake_minimum_required(VERSION 3.15)
project(xiaohuixiangqi VERSION 0.1.1)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)
set (CMAKE_CXX_STANDARD 20)

option(WITH_ELEEYE "Build EleEye engine" ON)

# Find the QtWidgets library
find_package(Qt6 COMPONENTS Widgets REQUIRED)

qt6_add_resources(QRC_FILES assets/assets.qrc)

set(xiaohuixiangqi_SRC
  src/main.cpp
  src/mainwindow.cpp
  src/board.cpp
  src/piece.cpp
  src/player.cpp
  src/gamestartdialog.cpp
  src/log.cpp
  src/record.cpp
  src/settingsdialog.cpp
  ${QRC_FILES}
)

# Tell CMake to create the helloworld executable
add_executable(xiaohuixiangqi ${xiaohuixiangqi_SRC})

# Use the Widgets module from Qt 6.
target_link_libraries(xiaohuixiangqi Qt6::Widgets)

if (WITH_ELEEYE)
    set(eleeye_SRC
      3rdparty/eleeye/pipe.cpp
      3rdparty/eleeye/ucci.cpp
      3rdparty/eleeye/pregen.cpp
      3rdparty/eleeye/position.cpp
      3rdparty/eleeye/genmoves.cpp
      3rdparty/eleeye/hash.cpp
      3rdparty/eleeye/book.cpp
      3rdparty/eleeye/movesort.cpp
      3rdparty/eleeye/preeval.cpp
      3rdparty/eleeye/evaluate.cpp
      3rdparty/eleeye/search.cpp
      3rdparty/eleeye/eleeye.cpp
    )

    add_executable(eleeye ${eleeye_SRC})
    target_compile_definitions(eleeye PRIVATE NDEBUG)
    target_compile_options(eleeye PRIVATE -O4 -Wall)
    target_include_directories(eleeye PRIVATE ${CMAKE_SOURCE_DIR}/3rdparty/eleeye/base)

    install(TARGETS eleeye DESTINATION bin)
endif()

# Install the executable
install(TARGETS xiaohuixiangqi DESTINATION bin)
install(FILES assets/xiaohuixiangqi.png DESTINATION share/icons/hicolor/512x512/apps/)
install(FILES assets/xiaohuixiangqi.desktop DESTINATION share/applications/)
