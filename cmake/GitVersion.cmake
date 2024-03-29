function(get_git_version config_file_in config_file_out)
    set(CACHE_FILE ${GIT_VERSION_TARGET_DIR}/git_state.txt)

    execute_process(
        COMMAND git log -1 --format=%h\;%at
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_INFO
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    LIST(GET GIT_INFO 0 GIT_HASH)
    LIST(GET GIT_INFO 1 GIT_TIME)

    if(NOT EXISTS ${GIT_VERSION_TARGET_DIR})
        file(MAKE_DIRECTORY ${GIT_VERSION_TARGET_DIR})
    endif()

    if(EXISTS ${CACHE_FILE})
        file(STRINGS ${CACHE_FILE} GIT_CACHE_CONTENT)
        LIST(GET GIT_CACHE_CONTENT 0 GIT_HASH_CACHE)
        LIST(GET GIT_CACHE_CONTENT 1 GIT_TIME_CACHE)
    endif()
    if(NOT DEFINED GIT_HASH_CACHE)
        set(GIT_HASH_CACHE "INVALID")
    endif()

    if(NOT ${GIT_HASH} STREQUAL ${GIT_HASH_CACHE} OR NOT EXISTS ${config_file_out})
        file(WRITE ${CACHE_FILE} "${GIT_HASH}\n${GIT_TIME}")
        file(REMOVE ${config_file_out})
        configure_file(${config_file_in} ${config_file_out} @ONLY)
    endif()
endfunction()

function(setup_git_version_config target config_file_in)
    set(GIT_VERSION_TARGET_DIR ${CMAKE_BINARY_DIR}/git_version)

    get_filename_component(abs_config_file_in ${config_file_in} ABSOLUTE)
    get_filename_component(config_file_name ${config_file_in} NAME_WLE)
    set(config_file_out ${GIT_VERSION_TARGET_DIR}/${config_file_name})

    add_custom_target(
        CheckGitVersion
        COMMAND ${CMAKE_COMMAND}
        -DRUN_GIT_VERSION_CHECK=1
        -DGIT_VERSION_TARGET_DIR=${GIT_VERSION_TARGET_DIR}
        -DGIT_VERSION_CONFIG_FILE_IN=${abs_config_file_in}
        -DGIT_VERSION_CONFIG_FILE_OUT=${config_file_out}
        -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/GitVersion.cmake
        COMMENT "Retrieve git info"
        BYPRODUCTS ${config_file_out}
    )

    add_dependencies(${target} CheckGitVersion)
    target_sources(${target} PRIVATE ${config_file_out})
    target_include_directories(${target} PRIVATE ${GIT_VERSION_TARGET_DIR})

    get_git_version(${abs_config_file_in} ${config_file_out})
endfunction()

if(RUN_GIT_VERSION_CHECK AND GIT_VERSION_CONFIG_FILE_IN AND GIT_VERSION_CONFIG_FILE_OUT)
    get_git_version(${GIT_VERSION_CONFIG_FILE_IN} ${GIT_VERSION_CONFIG_FILE_OUT})
endif()
