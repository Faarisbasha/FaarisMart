cmake_minimum_required(VERSION 3.20)

project(FaarisMart LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Thread support
find_package(Threads REQUIRED)

# PostgreSQL
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(LIBPQ libpq)
endif()

if(NOT LIBPQ_FOUND)
    find_package(PostgreSQL REQUIRED)
    set(LIBPQ_INCLUDE_DIRS ${PostgreSQL_INCLUDE_DIRS})
    set(LIBPQ_LIBRARIES ${PostgreSQL_LIBRARIES})
    set(LIBPQ_LIBRARY_DIRS ${PostgreSQL_LIBRARY_DIRS})
endif()

# -------------------------------
# API Server (httplib)
# -------------------------------
add_executable(FaarisAPI
    api.cpp
)

target_include_directories(FaarisAPI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBPQ_INCLUDE_DIRS}
)

target_link_directories(FaarisAPI PRIVATE
    ${LIBPQ_LIBRARY_DIRS}
)

target_link_libraries(FaarisAPI PRIVATE
    ${LIBPQ_LIBRARIES}
    Threads::Threads
)

# Windows-only networking library
if(WIN32)
    target_link_libraries(FaarisAPI PRIVATE ws2_32)
endif()

# -------------------------------
# Console Application (Windows only, uses <conio.h>)
# -------------------------------
if(WIN32)
    add_executable(FaarisMart
        main.cpp
        database.cpp
        product.cpp
        cart.cpp
        checkout.cpp
        admin.cpp
        seller.cpp
        order.cpp
        review.cpp
    )

    target_include_directories(FaarisMart PRIVATE
        ${LIBPQ_INCLUDE_DIRS}
    )

    target_link_directories(FaarisMart PRIVATE
        ${LIBPQ_LIBRARY_DIRS}
    )

    target_link_libraries(FaarisMart PRIVATE
        ${LIBPQ_LIBRARIES}
    )
endif()