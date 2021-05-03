aux_source_directory(. DIR_SRCS)
add_executable(equinox ${DIR_SRCS})

target_link_libraries(equinox m)
