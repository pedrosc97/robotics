#!/bin/sh

if [ -n "$DESTDIR" ] ; then
    case $DESTDIR in
        /*) # ok
            ;;
        *)
            /bin/echo "DESTDIR argument must be absolute... "
            /bin/echo "otherwise python's distutils will bork things."
            exit 1
    esac
    DESTDIR_ARG="--root=$DESTDIR"
fi

echo_and_run() { echo "+ $@" ; "$@" ; }

echo_and_run cd "/home/pedrosc/robotics/src/cv_bridge"

# ensure that Python install destination exists
echo_and_run mkdir -p "$DESTDIR/home/pedrosc/robotics/install/lib/python2.7/dist-packages"

# Note that PYTHONPATH is pulled from the environment to support installing
# into one location when some dependencies were installed in another
# location, #123.
echo_and_run /usr/bin/env \
    PYTHONPATH="/home/pedrosc/robotics/install/lib/python2.7/dist-packages:/home/pedrosc/robotics/build/lib/python2.7/dist-packages:$PYTHONPATH" \
    CATKIN_BINARY_DIR="/home/pedrosc/robotics/build" \
    "/usr/bin/python" \
    "/home/pedrosc/robotics/src/cv_bridge/setup.py" \
    build --build-base "/home/pedrosc/robotics/build/cv_bridge" \
    install \
    $DESTDIR_ARG \
    --install-layout=deb --prefix="/home/pedrosc/robotics/install" --install-scripts="/home/pedrosc/robotics/install/bin"
