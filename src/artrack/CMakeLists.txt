cmake_minimum_required(VERSION 2.8.3)
project(artrack)
find_package(catkin REQUIRED COMPONENTS
	roscpp 
	rospy
	std_msgs
	message_generation
	image_transport
	geometry_msgs
	ardrone_autonomy
	uvc_camera
	sensor_msgs
	cv_bridge
	keyboard_reader
)

find_package(OpenCV 3.4.5 REQUIRED COMPONENTS
	opencv_core
	opencv_imgproc
	opencv_highgui
	opencv_dnn
)

set(OpenCV_INCLUDE_DIRS
	/usr/local/include/opencv2
)

include_directories(
    ${catkin_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

add_executable(artrack_imgproc src/artrack_imgproc.cpp)
target_link_libraries(artrack_imgproc 
	${catkin_LIBRARIES}
	${OpenCV_LIBRARIES}
)
set_target_properties(artrack_imgproc PROPERTIES RUNTIME_OUTPUT_DIRECTORY ~/robotics/src/artrack/src CXX_STANDARD 11)

add_executable(artrack_keyinput src/artrack_keyinput.cpp)
target_link_libraries(artrack_keyinput 
	${catkin_LIBRARIES}
)
set_target_properties(artrack_keyinput PROPERTIES RUNTIME_OUTPUT_DIRECTORY ~/robotics/src/artrack/src CXX_STANDARD 11)

add_executable(artrack_controller src/artrack_controller.cpp)
target_link_libraries(artrack_controller
	${catkin_LIBRARIES}
)
set_target_properties(artrack_controller PROPERTIES RUNTIME_OUTPUT_DIRECTORY ~/robotics/src/artrack/src CXX_STANDARD 11)

catkin_package(CATKIN_DEPENDS
	message_runtime
	sensor_msgs
	image_transport
	cv_bridge
	geometry_msgs
)