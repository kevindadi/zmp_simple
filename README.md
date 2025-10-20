# Install

ipc 通信默认绑定地址"/tmp/docker_share"
mkdir /tmp/docker_share

## 依赖

apt-get install -y build-essential cmake pkg-config python3 python3-dev pybind11-dev

## json 支持需要单独安装

cd json-3.12.0 && cmake -Bbuild -H && cd build && sudo make install

## 源码构建

cmake -Bbuild -H.
cmake --build build

## docker 支持

cd docker && docker-compose up --build
