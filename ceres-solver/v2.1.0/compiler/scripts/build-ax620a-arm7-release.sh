#!/usr/bin/env bash


# I_BUILD_VRESION="2.4.13.4"
# I_BUILD_VRESION="4.7.0"
# I_BUILD_VRESION="4.7.0"
I_BUILD_VRESION="4.6.0"

URL_OPENCV="https://github.com/opencv/opencv/archive/refs/tags/$I_BUILD_VRESION.zip"
URL_OPENCV_CONTRIB="https://github.com/opencv/opencv_contrib/archive/refs/tags/$I_BUILD_VRESION.zip"


ZIP_OPENCV="opencv-$I_BUILD_VRESION.zip"
ZIP_OPENCV_CONTRIB="opencv_contrib-$I_BUILD_VRESION.zip"

# rm -rf "$ZIP_OPENCV"
# rm -rf "$ZIP_OPENCV_CONTRIB"

rm -rf "opencv-$I_BUILD_VRESION"
rm -rf "opencv_contrib-$I_BUILD_VRESION"

if [ -f "$ZIP_OPENCV" ]
then
    echo "$ZIP_OPENCV File exists."
else
    wget -O "$ZIP_OPENCV" "$URL_OPENCV"
fi

if [ -f "$ZIP_OPENCV_CONTRIB" ]
then
    echo "$ZIP_OPENCV_CONTRIB File exists."
else
    wget -O "$ZIP_OPENCV_CONTRIB" "$URL_OPENCV_CONTRIB"
fi


unzip -q "$ZIP_OPENCV"
unzip -q "$ZIP_OPENCV_CONTRIB"

DIR_INSTALL="../../install"
DIR_CONTRIB="../../opencv_contrib-$I_BUILD_VRESION/modules"
TOOLCHAIN="../cmake/arm-linux-arm7-aixin.toolchain.cmake"

# shellcheck disable=SC2046
./build.sh  -r   -v "$I_BUILD_VRESION" -a $(cat ax620a_options.txt)  -i  "$DIR_INSTALL"  -e "$DIR_CONTRIB"  -t  9
