cmake_minimum_required(VERSION 3.20)

project(PUJOgreTest)

set(CMAKE_CXX_STANDARD 17)

## -- Packages
find_package(OGRE REQUIRED)

## -- Targets
subdirs(src)


#configure_file(demo/data/resources.cfg.in ${CMAKE_BINARY_DIR}/resources.cfg @ONLY)
## copy essential config files next to our binary where OGRE autodiscovers them
#file(COPY ${OGRE_CONFIG_DIR}/plugins.cfg DESTINATION ${CMAKE_BINARY_DIR})

## eof - CMakeLists.txt