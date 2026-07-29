cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(cuktech_ble)

# Merge bootloader + partition-table + app into a single firmware.bin
# Bootloader offset: 0x1000 for ESP32, 0x0 for others (C3/S3)
if(${IDF_TARGET} STREQUAL "esp32")
    set(_BOOT_OFFSET "0x1000")
else()
    set(_BOOT_OFFSET "0x0")
endif()

if(NOT TARGET merge-bin)
add_custom_target(merge-bin
    COMMAND python -m esptool --chip ${IDF_TARGET} merge_bin
        -o ${CMAKE_BINARY_DIR}/firmware.bin
        ${_BOOT_OFFSET} ${CMAKE_BINARY_DIR}/bootloader/bootloader.bin
        0x8000 ${CMAKE_BINARY_DIR}/partition_table/partition-table.bin
        0x10000 ${CMAKE_BINARY_DIR}/cuktech_ble.bin
    COMMENT "Merging bootloader+partition+app into firmware.bin"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
endif()
