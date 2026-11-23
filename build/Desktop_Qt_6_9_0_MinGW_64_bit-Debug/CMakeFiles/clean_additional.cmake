# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\sqlwork_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\sqlwork_autogen.dir\\ParseCache.txt"
  "sqlwork_autogen"
  )
endif()
