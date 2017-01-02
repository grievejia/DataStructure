cmake_minimum_required(VERSION 3.1)
project(DataStructure)

# Set compilation flag
set (CMAKE_CXX_FLAGS "-Wall -fno-rtti")
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set (CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native -mtune=native -DNDEBUG")

include(CTest)

set(HEADER_PATH ${PROJECT_SOURCE_DIR}/include)
include_directories (${HEADER_PATH})
add_library (ds STATIC
	lib/SmallVector.cpp
	lib/StringView.cpp
)
set_property(TARGET ds PROPERTY CXX_STANDARD 14)
set_property(TARGET ds PROPERTY CXX_STANDARD_REQUIRED ON)

install (
	DIRECTORY ${HEADER_PATH}/DataStructure
	DESTINATION include
)
install (
	TARGETS ds
	DESTINATION lib
)
target_include_directories(ds PUBLIC
  $<BUILD_INTERFACE:${HEADER_PATH}>
  $<INSTALL_INTERFACE:include>)

if (BUILD_TESTING)
	enable_testing()
	set(UNITTEST_PATH ${PROJECT_SOURCE_DIR}/unittest)

	set(GTEST_INSTALL_PATH ${PROJECT_BINARY_DIR}/gtest)
	include(ExternalProject)
	ExternalProject_Add(
		googletest
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG master
		INSTALL_DIR ${GTEST_INSTALL_PATH}
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
	)
	include_directories(${GTEST_INSTALL_PATH}/include)
	link_directories(${GTEST_INSTALL_PATH}/lib)

	macro(add_unit_test testname)
		add_executable(${testname} ${UNITTEST_PATH}/${testname}.cpp)
		add_dependencies(${testname} googletest)
		target_link_libraries(${testname} ds gtest gtest_main pthread)
    set_property(TARGET ${testname} PROPERTY CXX_STANDARD 14)
    set_property(TARGET ${testname} PROPERTY CXX_STANDARD_REQUIRED ON)
		add_test(${testname} ${testname})		
	endmacro()

	add_unit_test(ArrayRefTest)
	add_unit_test(DenseMapTest)
	add_unit_test(DenseSetTest)
	add_unit_test(DynamicBitSetTest)
	add_unit_test(FlatSetTest)
	add_unit_test(SmallVectorTest)
	add_unit_test(StringMapTest)
	add_unit_test(StringViewTest)
	add_unit_test(UnorderedCollectionTest)
	add_unit_test(VectorMapTest)
	add_unit_test(VectorSetTest)
	add_unit_test(WorkListTest)
endif()
