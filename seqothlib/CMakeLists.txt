set(libL2Node_SRCS
    L2Node.hpp
    L2Node.cpp
)

set (libUtil_SRCS
    util.h
    util.cpp
)

set (libL1Node_SRCS
    L1Node.hpp
    L1Node.cpp
    )
# Declare the library

add_library(libUtil STATIC ${libUtil_SRCS})

add_library(libL2Node STATIC ${libL2Node_SRCS}) 

add_library(libL1Node STATIC ${libL1Node_SRCS}) 

TARGET_LINK_LIBRARIES(libL2Node libUtil tinyxml2 z)

TARGET_LINK_LIBRARIES(libL1Node libUtil tinyxml2 z pthread)

# Specify here the include directories exported
# by this library
include_directories(libL2Node PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../lib/
)

include_directories(libL1Node PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ../lib/
)
