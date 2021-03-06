link_libraries(common_base)

add_executable(name_server
    ${PACKAGE_SOURCE_ROOT}/server/main_ns.cpp
    ${PACKAGE_SOURCE_ROOT}/server/CNameServer.cpp
    ${PACKAGE_SOURCE_ROOT}/server/CInterNameProxy.cpp
    ${PACKAGE_SOURCE_ROOT}/server/CHostProxy.cpp
    ${PACKAGE_SOURCE_ROOT}/security/CServerSecurityConfig.cpp
)

add_executable(host_server
    ${PACKAGE_SOURCE_ROOT}/server/main_hs.cpp
    ${PACKAGE_SOURCE_ROOT}/server/CHostServer.cpp
    ${PACKAGE_SOURCE_ROOT}/security/CHostSecurityConfig.cpp
)

add_executable(lssvc
    ${PACKAGE_SOURCE_ROOT}/server/main_ls.cpp
)

add_executable(lshost
    ${PACKAGE_SOURCE_ROOT}/server/main_lh.cpp
)

add_executable(logsvc
    ${PACKAGE_SOURCE_ROOT}/server/main_log_server.cpp
    ${PACKAGE_SOURCE_ROOT}/server/CLogPrinter.cpp
)

add_executable(logviewer
    ${PACKAGE_SOURCE_ROOT}/server/main_log_client.cpp
    ${PACKAGE_SOURCE_ROOT}/server/CLogPrinter.cpp
)

install(TARGETS name_server host_server lssvc lshost logsvc logviewer RUNTIME DESTINATION usr/bin)
