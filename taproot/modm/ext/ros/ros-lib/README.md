# ROS Library for modm

Repository of generated messages headers for rosserial.

Normally, a pipeline schedule of GitLab's CI updates this repository. If more message headers are required the ROS package can be added to `package.list`. The CI will then check in the generated files afterwards.

These files can be generated manually with `./rosserial_mbed/src/rosserial_mbed/make_libraries.py` from `jade-devel` branch in rosserial.

## Build Architecture

The repository is primarily maintained at github.com (https://github.com/modm-io/ros-lib).

It is automatically mirrored to gitlab.com (https://gitlab.com/modm-io/ros-lib).

A pipeline schedule at GitLab (https://gitlab.com/modm-io/ros-lib/pipeline_schedules) runs the `.gitlab-ci.yaml` once a week.

In this pipeline, the Docker image is built in the GitLab CI, started and `build.bash` and `deploy.bash` are run.

GitLab has a deploy SSH key of GitHub so any changes which were introduced by the build process are committed and pushed to the GitHub repository.

Finally, the `ros-lib` submodule of `modm-io` is manually bumped to a new version by one of the modm maintainers.

## Manual Build Process

In case you want or need to build the messages manually locally:

Clone the repository:

    git clone https://github.com/modm-io/ros-lib
    cd ros-lib

Build the Docker image:

    docker build -t ros-lib:latest .

Run the image and mount the repository to `/build`:

    docker run -it --mount type=bind,source="$(pwd)",target=/build ros-lib:latest /bin/bash

Run the `build.bash` from the repository:

	cd /build
    build.bash
