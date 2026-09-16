cmake_minimum_required(VERSION 3.20)

project(FaarisMart LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
# Use MSYS2's pkg-config to find libpq
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQ REQUIRED libpq)

target_include_directories(FaarisMart PRIVATE ${LIBPQ_INCLUDE_DIRS})
target_link_directories(FaarisMart PRIVATE ${LIBPQ_LIBRARY_DIRS})
target_link_libraries(FaarisMart PRIVATE ${LIBPQ_LIBRARIES})