# AegisServer

AegisServer 是一个基于 C++17 / Linux / epoll 的高并发静态资源 WebServer，重点围绕静态文件高效分发、连接治理与基础可观测性进行设计与实现。

## 特性

- 基于 epoll 的事件驱动模型
- 非阻塞 socket
- 持久连接对象 HttpConn
- Buffer 管理读写状态
- HTTP/1.1 GET 支持
- keep-alive 连接复用
- 静态资源分发
- 路径安全校验（目录穿越拦截）
- /metrics 指标接口
- 单 IP 最大连接数限制
- 空闲连接超时回收
- 线程池处理请求解析与响应构造
- mmap + writev 聚合发送静态文件

## 构建

```bash
mkdir build
cd build
cmake ..
make -j

运行

在项目根目录执行：
./build/aegis_server

配置
配置文件位于：
config/server.conf

支持配置：
listen_ip
listen_port
thread_pool_size
idle_timeout_ms
max_connections_per_ip
static_root
Benchmark

使用 ab：
./benchmark/ab_test.sh 127.0.0.1 8080 / 1000 50

使用 wrk：
wrk -t4 -c100 -d10s -s benchmark/wrk.lua http://127.0.0.1:8080

- 基于 IP 的固定窗口限流
- 扩展指标统计：
  - total_requests
  - successful_requests
  - not_found_requests
  - forbidden_requests
  - too_many_requests
  - timeout_closed
  - keep_alive_hits
  - bytes_sent