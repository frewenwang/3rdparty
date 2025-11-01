# OpenCV

## OpenCV4.7

下载地址：https://github.com/opencv/opencv/releases/tag/4.7.0

源码上传git时候，opencv-4.7.0-ios-framework.zip文件太大，无法上传

```
remote: error: File: d126d8166daebdda0603e2320a216f19e1efa043 120.74 MB, exceeds 100.00 MB.
remote: Use command below to see the filename:
remote: git rev-list --objects --all | grep d126d8166daebdda0603e2320a216f19e1efa043
remote: Please remove the file from history and try again. (https://gitee.com/help/articles/4232)
To gitee.com:frewen1225/opencv-library.git
 ! [remote rejected] master -> master (pre-receive hook declined)
error: failed to push some refs to 'gitee.com:frewen1225/opencv-library.git'
(base) ➜  opencv-library git:(master) git rev-list --objects --all | grep d126d8166daebdda0603e2320a216f19e1efa043
d126d8166daebdda0603e2320a216f19e1efa043 opencv4.7.0/opencv-4.7.0-ios-framework.zip
```

暂时删除。

下载地址：https://github.com/opencv/opencv/releases/tag/4.7.0

## OpenCV4.8.1

下载地址：https://github.com/opencv/opencv/releases/tag/4.8.1

下载地址的共有文件：

```
opencv-4.8.1-android-sdk.zip

opencv-4.8.1-ios-framework.zip

opencv-4.8.1-windows.exe

Source code (zip)
Source code (tar.gz)
```

当我们将源码备份上传的时候，opencv-4.7.0-ios-framework.zip文件太大，无法上传

```shell
remote: error: File: d126d8166daebdda0603e2320a216f19e1efa043 120.74 MB, exceeds 100.00 MB.
remote: Use command below to see the filename:
remote: git rev-list --objects --all | grep d126d8166daebdda0603e2320a216f19e1efa043
remote: Please remove the file from history and try again. (https://gitee.com/help/articles/4232)
To gitee.com:frewen1225/opencv-library.git
 ! [remote rejected] master -> master (pre-receive hook declined)
error: failed to push some refs to 'gitee.com:frewen1225/opencv-library.git'
(base) ➜  opencv-library git:(master) git rev-list --objects --all | grep d126d8166daebdda0603e2320a216f19e1efa043
d126d8166daebdda0603e2320a216f19e1efa043 opencv4.7.0/opencv-4.7.0-ios-framework.zip
```

暂时删除。

我们暂时只将源码上传


# 常见问题

## 问题一：

```
opencv库 交叉编译features.h:272:52: error: operator ‘&&‘ has no right operand
```

解决方法：https://blog.csdn.net/amxld/article/details/111565705

```
ARM交叉编译Opencv库遇到的问题

opencv库 交叉编译features.h:272:52: error: operator ‘&&‘ has no right operand
我找到该文件 272行是下面这句
#if defined _FILE_OFFSET_BITS && _FILE_OFFSET_BITS == 64

在#if defined前面 加上
#define _FILE_OFFSET_BITS 64

下面的问题转载至 https://zhuanlan.zhihu.com/p/84839635
可能的错误
common.cc:52:2: error: #error “No suitable threading library available.” #error "No suitable threading library available.

解决方法：
在/home/jsaon/tool/opencv-3.4.5/3rdparty/protobuf/src/google/protobuf/stubs/common.cc 中添加下面这一行
#define HAVE_PTHREAD
```


## 问题二：

```
libpthread.so.0: error adding symbols: DSO missing from command line
```

解决方法：https://stackoverflow.com/questions/19901934/libpthread-so-0-error-adding-symbols-dso-missing-from-command-line

```
# e.g. with C++ 17, change to other version if you need
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread")
```
