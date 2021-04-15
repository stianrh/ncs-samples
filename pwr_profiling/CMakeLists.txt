cmake_minimum_required(VERSION 3.13.4)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(hci_rpmsg_CONF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/hci_rpmsg.conf)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project (pwr_profiling)

# add sources
target_sources(app PRIVATE src/main.c)

zephyr_include_directories(src)
