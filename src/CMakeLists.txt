add_library(GoLife golife.h golife.cxx)
target_link_libraries(GoLife
    PUBLIC
    # cxx_project_warnings
    cxx_project_options
)

add_executable(game-of-life main.cxx)
target_link_libraries(game-of-life
    PUBLIC
    GoLife
)
