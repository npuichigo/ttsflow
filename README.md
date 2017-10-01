# ttsflow

speech synthesis c++ inference for voicenet

**ttsflow** can be built with both google's bazel build tool and gnu's make.
We provide compatibility with bazel for convenience to build from scratch
once cloned. However, the final executable file may be really big, which
is nearly 60M. The reason is that many unused tensorflow codes are
compiled and linked into the final executable file staticly. Maybe it is
more suitable to build with gnu's makefile, especially when you want to
incorporate tensorflow into your project, so we provide makefile too.

## Installation

### build with bazel

It's quite simple to build this project with bazel, just type:
```shell
cd ./third_party/tensorflow
./configure
cd ../../
bazel build -c opt --config=opt ttsflow/examples/...
```

The generated executable file is located in bazel-bin/ttsflow/examples/ttsflow_test

### build with gnu's make

In order to build with tensorflow, you should first compile tensorflow
as dynamic-link library.

To build libtensorflow_cc.so for c++, just run:
```shell
cd ./third_party/tensorflow
./configure
bazel build -c opt --config=opt //tensorflow:libtensorflow_cc.so
```

Another prerequisite is protobuf, which should be installed in ttsflow-gnu-make/third_party/protobuf:
```
cd ./third_party/protobuf
./autogen.sh
./configure --prefix=`pwd`
make
make install
```

To compile this project, just directly type make, and a symbolic link named main.bin will
be created under the root dir.
```shell
make
./ttsflow_main
```

Also, you can run the test cases after compiling the test files.
```shell
make test
```
