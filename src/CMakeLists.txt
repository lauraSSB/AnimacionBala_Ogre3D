add_executable(
  myFirstOGREApp
  Application.cxx
  main.cxx
  )
target_link_libraries(
  myFirstOGREApp
  stdc++fs
  OgreMain OgreBites
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/resources.cfg
  ${CMAKE_CURRENT_BINARY_DIR}/resources.cfg
  @ONLY
  )

add_custom_target(myFirstOGREApp_models ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/models ${CMAKE_CURRENT_BINARY_DIR}/models
  DEPENDS myFirstOGREApp
  )

## eof - CMakeLists.txt
