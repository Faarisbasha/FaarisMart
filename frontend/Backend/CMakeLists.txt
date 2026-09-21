cmake_minimum_required(VERSION 3.20)

project(FaarisMart LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# PostgreSQL
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQ REQUIRED libpq)

# -------------------------------
# Console Application
# -------------------------------
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

target_include_directories(FaarisMart PRIVATE ${LIBPQ_INCLUDE_DIRS})
target_link_directories(FaarisMart PRIVATE ${LIBPQ_LIBRARY_DIRS})
target_link_libraries(FaarisMart PRIVATE ${LIBPQ_LIBRARIES})

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
target_link_libraries(FaarisAPI PRIVATE
    ${LIBPQ_LIBRARIES}
    ws2_32
)