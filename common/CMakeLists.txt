add_library(oj_common STATIC
    src/log.cpp
    src/config.cpp
    src/mysql_pool.cpp
    src/redis_cache.cpp
    src/json_error.cpp
    src/bootstrap.cpp
)

target_include_directories(oj_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(oj_common PUBLIC Threads::Threads)

if(OJ_ENABLE_MYSQL)
    target_compile_definitions(oj_common PUBLIC OJ_WITH_MYSQL)
    if(DEFINED MYSQL_CONCPP_INCLUDE)
        target_include_directories(oj_common PRIVATE ${MYSQL_CONCPP_INCLUDE})
    endif()
    if(DEFINED MYSQL_CONCPP_LIB)
        target_link_libraries(oj_common PRIVATE ${MYSQL_CONCPP_LIB})
    else()
        find_library(MYSQL_CPPCONN_LIB
            NAMES mysqlcppconn mysqlcppconn8 libmysqlcppconn
            PATHS ${MYSQL_CONCPP_ROOT}/lib64 ${MYSQL_CONCPP_ROOT}/lib
        )
        if(MYSQL_CPPCONN_LIB)
            target_link_libraries(oj_common PRIVATE ${MYSQL_CPPCONN_LIB})
        else()
            message(WARNING "OJ_ENABLE_MYSQL=ON but MYSQL_CONCPP_LIB not found; link manually")
        endif()
    endif()
endif()

if(OJ_ENABLE_REDIS)
    target_compile_definitions(oj_common PUBLIC OJ_WITH_REDIS)
    find_package(hiredis CONFIG QUIET)
    if(hiredis_FOUND)
        target_link_libraries(oj_common PRIVATE hiredis::hiredis)
    else()
        find_library(HIREDIS_LIB NAMES hiredis)
        if(HIREDIS_LIB)
            target_link_libraries(oj_common PRIVATE ${HIREDIS_LIB})
        else()
            message(WARNING "OJ_ENABLE_REDIS=ON but hiredis not found; link manually")
        endif()
    endif()
endif()
