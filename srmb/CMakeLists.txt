cmake_minimum_required(VERSION 3.0.2)
project(srmb)

add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS  
  roscpp  
  std_msgs
  message_generation
  message_runtime
)  

generate_messages(DEPENDENCIES
  std_msgs
)


catkin_package(  
 INCLUDE_DIRS include  
 CATKIN_DEPENDS roscpp std_msgs 
)  
 
include_directories(  
  include 
  ${catkin_INCLUDE_DIRS}  
)  




add_library(lib_srmb  
  include/srmb.h  
  src/srmb.cpp  
)  
# 为库文件添加依赖项  
add_dependencies(lib_srmb ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})  















# 指出可执行文件所在路径并且给可执行文件取名为srmbtestsub  
add_executable(srmbtestsub src/srmbtestsub.cpp ) 
# 给srmbtestsub所代表的这些需要编译为可执行文件的源文件添加依赖项  
add_dependencies(srmbtestsub ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})  
# 给库文件和需要编译的源文件链接catkin官方共享库  
target_link_libraries(srmbtestsub  lib_srmb  
  ${catkin_LIBRARIES}  
)  




# 指出可执行文件所在路径并且给可执行文件取名为srmbtestpub 
add_executable(srmbtestpub src/srmbtestpub.cpp)  
# 给srmbtestpub所代表的这些需要编译为可执行文件的源文件添加依赖项  
add_dependencies(srmbtestpub ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})  
# 给库文件和需要编译的源文件链接catkin官方共享库  
target_link_libraries( srmbtestpub lib_srmb  
  ${catkin_LIBRARIES}  
)  
