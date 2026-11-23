add_executable(
  animacionBalaApp
  Application.cxx
  PhysicsModel.cxx
  CameraManager.cxx
  ShootController.cxx
  ShootMeterView.cxx
  main.cxx
  )
target_link_libraries(
  animacionBalaApp
  stdc++fs
  OgreMain OgreBites OgreBullet
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/resources.cfg
  ${CMAKE_CURRENT_BINARY_DIR}/resources.cfg
  @ONLY
  )

add_custom_target(animacionBalaApp_models ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_CURRENT_SOURCE_DIR}/models ${CMAKE_CURRENT_BINARY_DIR}/models
    DEPENDS animacionBalaApp
  )

## eof - CMakeLists.txt