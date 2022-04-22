#!/usr/bin/env bash

# Exit on first error
set -e

source /opt/ros/$ROS_DISTRO/setup.bash
source /catkin_ws/devel/setup.bash

# Remove generated files as make_libraries does not overwrite files
rm -rf ros_lib

# Generate message fiels
rosrun rosserial_mbed make_libraries.py .

# Remove unused files before committing
rm -rf ros_lib/BufferedSerial*
rm ros_lib/MbedHardware.h
rm ros_lib/ros.h
rm -rf ros_lib/rosserial_arduino
rm -rf ros_lib/rosserial_mbed

# Remove trailing whitespaces
find . -type f -name '*.h' -exec sed --in-place 's/[[:space:]]\+$//' {} \+

# Replace 1e9 and 1e-9 with double(...)
find . -type f \( -name 'duration.h' -o -name 'time.h' \) -exec sed --in-place --regexp-extended 's/(1e9|1e-9)/double(\1)/g' {} \+

git status
