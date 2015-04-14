# Copyright: (C) 2012 IITRBCS
# Authors: Elena Ceseracciu
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT

FILE(GLOB globbedHeaders RELATIVE /home/cfang/walkman/external/GazeboYARPPlugins/_yarp_idl_/thrift/objects_rpc/ "/home/cfang/walkman/external/GazeboYARPPlugins/_yarp_idl_/thrift/objects_rpc/*.h")
FILE(GLOB globbedSources RELATIVE /home/cfang/walkman/external/GazeboYARPPlugins/_yarp_idl_/thrift/objects_rpc/ "/home/cfang/walkman/external/GazeboYARPPlugins/_yarp_idl_/thrift/objects_rpc/*.cpp")

set(headers)
set(sources)

# check that globbed files are really the ones we need from generated listfile
FILE(STRINGS /home/cfang/walkman/external/GazeboYARPPlugins/_yarp_idl_/thrift/objects_rpc/objects_rpc_indexALL.txt index)
#prepare include and src directories in the source tree
make_directory("/home/cfang/walkman/external/GazeboYARPPlugins/autogenerated/include/thrift")
make_directory("/home/cfang/walkman/external/GazeboYARPPlugins/autogenerated/src/thrift/")
foreach(genFile ${index})
  get_filename_component(type ${genFile} EXT)
  get_filename_component(main ${genFile} NAME_WE)
  if (${type} STREQUAL ".h")
    list(FIND globbedHeaders ${genFile} inclFound)
    if (${inclFound} EQUAL -1)
      message(WARNING "${genFile} not found in objects_rpc dir!")
    else(${inclFound} EQUAL -1) 
      FILE(COPY /home/cfang/walkman/external/GazeboYARPPlugins/_yarp_idl_/thrift/objects_rpc/${genFile} DESTINATION /home/cfang/walkman/external/GazeboYARPPlugins/autogenerated/include/thrift/)
      list(APPEND headers "include/thrift/${genFile}")
    endif(${inclFound} EQUAL -1)
    list(FIND globbedHeaders ${main}Reply${type} inclFound)
    if (NOT ${inclFound} EQUAL -1)
      FILE(COPY /home/cfang/walkman/external/GazeboYARPPlugins/_yarp_idl_/thrift/objects_rpc/${main}Reply${type} DESTINATION /home/cfang/walkman/external/GazeboYARPPlugins/autogenerated/include/thrift/)
      list(APPEND headers "include/thrift/${main}Reply${type}")
    endif(NOT ${inclFound} EQUAL -1)
  elseif(${type} STREQUAL ".cpp")
    list(FIND globbedSources ${genFile} srcFound)
    if (${srcFound} EQUAL -1)
      message(WARNING "${genFile} not found in objects_rpc dir!")
    else(${srcFound} EQUAL -1) 
      FILE(COPY /home/cfang/walkman/external/GazeboYARPPlugins/_yarp_idl_/thrift/objects_rpc/${genFile} DESTINATION /home/cfang/walkman/external/GazeboYARPPlugins/autogenerated/src/thrift/)
      list(APPEND sources "src/thrift/${genFile}")
    endif(${srcFound} EQUAL -1)

  else(${type} STREQUAL ".h")
message(WARNING "Filename extension of ${genFile} is neither .h nor .cpp")
  endif(${type} STREQUAL ".h")
endforeach(genFile)

file(WRITE /home/cfang/walkman/external/GazeboYARPPlugins/autogenerated/thrift_objects_rpc_thrift.cmake "## This is an automatically-generated file.\n## It could get re-generated if the ALLOW_IDL_GENERATION flag is on\n\nset(headers ${headers})\nset(sources ${sources})\n")
