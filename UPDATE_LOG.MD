### 版本 1.7.0 更新汇总

```text
1. 
```

### 版本 1.6.4 更新汇总

```text
1. 服务器正式开发, 配置文件接入, 运行参数接入
2. 引入POSIX线程池技术, 引入MySQL连接池技术
3. 确认服务器模型为: epoll + POSIX线程池 + MySQL连接池
4. 将松散的全局变量 serv_fd,ssl_serv_fd 等打包成 server_info_t 结构体
5. 移除松散杂乱的 mode_ssl_client, mode_server_client 等全局变量和枚举
6. 将客户端的 SSLMode 移动到配置文件结构体内部
7. 将运行参数划分为预处理参数和配置项参数, 程序先读取预处理参数, 然后读取配置文件, 最后读取配置项目参数
8. 移除客户端结束时主动向服务器发送的"FIN"心跳数据包, 对于客户端的断开连接, 统一由epoll处理
```

### 版本 1.6.5 更新汇总

```text
1.引入UUID,为客户端进行唯一性标识
2.完善了服务器清理无效客户端的功能
3.支持保存树莓派的实时数据到数据库
4.自定义客户端多开压力测试软件开发完成,模型 fork + exec
```

### 版本 1.6.4 更新汇总

```text
1. 服务器正式开发, 配置文件接入, 运行参数接入
2. 引入POSIX线程池技术, 引入MySQL连接池技术
3. 确认服务器模型为: epoll + POSIX线程池 + MySQL连接池
4. 将松散的全局变量 serv_fd,ssl_serv_fd 等打包成 server_info_t 结构体
5. 移除松散杂乱的 mode_ssl_client, mode_server_client 等全局变量和枚举
6. 将客户端的 SSLMode 移动到配置文件结构体内部
7. 将运行参数划分为预处理参数和配置项参数, 程序先读取预处理参数, 然后读取配置文件, 最后读取配置项目参数
8. 移除客户端结束时主动向服务器发送的"FIN"心跳数据包, 对于客户端的断开连接, 统一由epoll处理
```

### 版本 1.6.3 更新汇总

```text
1. 删除线程,客户端工作流程简洁化,单进程单线程,简化代码逻辑
2. 细化信号场景处理方法,sendFIN,shutdownLights,closeSocket,exitCleanup,reset
3. 删除RESET宏,作用太少,使用魔法数字代替
4. 优化了errno变量的处理
```

### 版本 1.6.2 更新汇总

```text
1. 将函数名称统一更改为小驼峰
2. 对一些单词拼写进行的改正
3. 删除DEBUGMODE全局变量,默认即调试模式(有终端输出),可以使用--daemon运行参数进入非调试模式
4. 失败重连改为指数补偿算法, 删除frectime,frecatps配置项
5. 配置文件内容项目修改/新增,caFile,pidFile,DAEMON
6. 运行参数修改/新增,与配置文件基本一致,定义运行参数优先级高于配置文件
7. 删除UCert,UKey配置项,客户端不再支持加载自己的证书密钥(仍然可以加载CA证书验证服务器),简化SSL连接
9. 删除严格模式概念,默认需要root权限
```

### 版本 1.6.1 更新汇总

```text
1. 大量重构文件名,变量名
2. 函数依据功能归类存放(文件)
3. 提取全局变量至global中,查找管理更加方便
4. 配置文件配置项目修改,去除CheckMe选项,增加SSLMODE选项
5. 安全模式下对ssl连接进行了更加精细的模式区分
6. 重写CMakeLists.txt文件,重新梳理依赖关系
7. 将发送数据的工作单独划分到一条线程
8. 信号精细处理(包括ALRM,HUP,PIPE,INT,QUIT,TERM,ABRT)
9. 新增调试模式,禁用守护进程化,日志输出到控制终端
```

### 版本 1.5.2 更新汇总

```text
1. 简化主函数
2. 增加了运行时日志提示细节
3. 修复了errno不及时清零的问题
4. 优化代码:将大量的配置文件选项相关的全局变量组装成结构体
5. 细化交互数据过程中的服务器响应的处理
6. 弃用了未使用的sundry库
7. 优化清理函数,减少程序退出时发生的段错误
8. 修复了鉴别服务器身份失败时继续发送数据的bug
```

### 版本 1.5 更新汇总

```text
1. 优化了帮助信息提示
2. 修复了因可能的测量误差而发出的高温湿度警告
3. 弃用了旧版库(msg,file,mysock)
4. 测试服务器在并发处理多客户端时,增加了客户端鉴别
5. 优化了提权逻辑
6. 优化了配置文件读取逻辑,提升了运行速度
```

### 版本 1.4 更新汇总(大更新)

```text
1. 修复了主函数参数bug
2. 优化了守护进程使用逻辑
3. 新增了"严格模式"
    a.检查PID文件
    b.使用SSL通信(基于OpenSSL-Dev库)
4. 提供测试使用的证书和密钥(文件夹keys/readme.txt)
5. 优化了测试服务器的收数据逻辑,加强了程序出错健壮性
6. 当发生错误等级高于ERROR时由直接退出改为自定义退出函数
7. 修复了配置文件读入时会对value转大写的bug,调整了单条配置的空间分配
```