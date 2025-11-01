

# gtest源码编译


### 编译Linux版本

```
# 进入到gtest的源码

mkdir build
cd  build 
cmake  -DCMAKE_INSTALL_PREFIX=./install .. 
make && make install
```



### 编译Qnx版本


```shell
mkdir gtest-build
cd gtest-build 
cmake ../googletest -DCMAKE_INSTALL_PREFIX=./install  -DCMAKE_TOOLCHAIN_FILE=./arm-unknown-nto-qnx700eabi.toolchain.cmake
make && make install
```



## 编译Mac版本

###

```
# 进入到gtest的源码

mkdir build
cd  build 
cmake  -DCMAKE_INSTALL_PREFIX=./install .. 
make && make install
```



