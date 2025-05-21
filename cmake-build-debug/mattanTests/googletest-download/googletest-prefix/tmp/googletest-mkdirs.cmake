# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-src"
  "/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-build"
  "/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-download/googletest-prefix"
  "/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-download/googletest-prefix/tmp"
  "/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-download/googletest-prefix/src/googletest-stamp"
  "/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-download/googletest-prefix/src"
  "/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-download/googletest-prefix/src/googletest-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-download/googletest-prefix/src/googletest-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/cs/usr/ellorw.nir/os-projects/OS_ex3/cmake-build-debug/mattanTests/googletest-download/googletest-prefix/src/googletest-stamp${cfgdir}") # cfgdir has leading slash
endif()
