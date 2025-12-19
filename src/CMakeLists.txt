SET(SRC_SOURCES
    threadpool.cpp
)

set(SRC_HEADERS
    threadpool.h
)

LIST(SORT SRC_SOURCES)

add_library(src ${SRC_SOURCES} ${SRC_HEADERS})
target_include_directories(src PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})