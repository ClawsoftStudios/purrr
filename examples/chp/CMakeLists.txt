add_executable(chp main.c)
target_link_libraries(chp purrr)
target_include_directories(chp PRIVATE ../)