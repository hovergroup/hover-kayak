# ---------------------------------------------
# hovergroup_proto
#        File: CMakeLists.txt
#  Created on: Sep 10, 2013
#      Author: Josh Leighton
# ---------------------------------------------

#file(GLOB PROTOS ./*.proto)
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTOS})
file(GLOB PROTO_SRCS ./*.pb.cc)
file(GLOB PROTO_HDRS ./*.pb.h)
add_library(hovergroup_proto ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(hovergroup_proto ${PROTOBUF_LIBRARY})