cmake_minimum_required(VERSION 3.16)

project(Hue)

include_directories(../lib/mlx90640-library/headers)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/..)
set(MYMLXLIB ../lib/mlx90640-library/functions)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# find_package(Qt5 COMPONENTS Widgets Gui REQUIRED)

	
add_executable(sensor
		test.cpp 
		mlx90640_driver.cpp) 


# target_link_libraries(Hue PRIVATE cpr)

# target_link_libraries(sensor Qt5::Widgets)
# target_link_libraries(sensor Qt5::Gui)
# target_link_libraries(sensor qwt-qt5)

# install(TARGETS sensor)


# this project has dependencies on cpr and curl libs.
# if you have make installed cpr locally, use the following instead of fetch content.

# local use
# link_libraries(/usr/local/lib/libcpr.so)
# include_directories(/usr/local/include)

# fetch remotely
# include(FetchContent)
# FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
#                        GIT_TAG beb9e98806bb84bcc130a2cebfbcbbc6ce62b335) #1.7.2
# FetchContent_MakeAvailable(cpr)


