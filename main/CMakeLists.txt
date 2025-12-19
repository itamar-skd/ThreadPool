SET(MAIN_SOURCES
    main.cpp
)

LIST(SORT MAIN_SOURCES)

add_executable(threadpool main.cpp)

target_link_libraries(threadpool
    PRIVATE    
        src
)
target_include_directories(threadpool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})