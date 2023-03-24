### 原创声明

```text
本工程是本人由于兴趣在大学二,三年级学习了《TCP/IP网络编程》的全部章节
和《UNIX环境高级编程》的部分章节 (第1~4章,第7~11章和第13章)
后一点一点手写慢慢调试起来的,非百度粘贴~~~
```

### 简介

```text
纯C语言的Unix学习型项目,学习C,网络编程和内核编程的童鞋不要错过哦~
编写于树莓派4b,基于wiringPi库的数据实时检测采集并上传指定服务器的客户端
现实意义:将树莓派放置于停车位附近,根据距离判定车位是否空闲

服务器端还在开发中(预计23年06月前完成)...
但我们也准备了一个简单的基于select()的并发测试服务器: test_server.c
```

### 没有树莓派?

```text
这意味着您可能无法使用wiringPi库,不过我们贴心地准备了模拟器模式
它们的区别仅仅在于使用模拟器模式后raspi_drive.c文件中原本直接操作物理引脚电平获取数据的方式更改为获取随机数,没有其他影响
模式切换方式: 在CMakeLists.txt文件中找到set(SIMULATOR false),将false更改为true即可
```

### 安装

```shell
# 使用了cmake工具,操作极其简单
# 如果没有cmake工具或者wiringPi库,自行百度安装

# 下载工程
$ git clone https://gitee.com/Einc/raspi.git

# 编译
$ cd raspi && mkdir build && cd build && cmake .. && make
#或者
$ cd raspi && cmake -bindPort build && cmake --build build

# 安装(可选)
$ sudo cmake --install build || make install
```

### 使用

```text
[遇到问题] 在使用过程中,最有可能需要修改/查看以下文件:
CMakeLists.txt(修改编译模式)
rain.conf(配置文件,初次运行客户端会自动生成,用于客户端设置)
test_server.c(修改服务器密钥证书的路径)
keys/readme.txt(密钥和证书生成步骤)

灯光解释:
红灯亮起:成功与服务器建立TCP链接
绿灯亮起:超声波传感器返回小于30cm(满足物理条件:车位非空闲)
黄灯亮起:环境温湿度异常,不适合系统继续运行
黄灯闪烁:每闪烁一次表示上传一次数据

[注意] 请保证客户端和服务器在同一模式运行(不能一个普通模式,另外一个严格模式)
```

```shell
# 调试模式
$ cd <你的构建目录>
# 常规
$ sudo bin/raind 
# 参数一: 守护进程
$ sudo bin/raind --daemon
# 参数二: 严格模式
$ sudo bin/raind --strict
# 参数三: 只删除PID文件(不可与其他参数混用,因为它清理后立即退出)
$ sudo bin/raind --clean
# 参数四: 刷新配置文件,使用默认的配置文件覆盖现有配置文件(无论是否存在)
$ sudo bin/raind --default-conf
# 参数五: 快捷打开配置文件
$ sudo bin/raind --settings
# 参数六: 帮助信息
$ sudo bin/raind --help

# 服务模式
# 启动
$ sudo systemctl start raind
# 停止
$ sudo systemctl stop raind
# 开机自启(可选)
$ sudo systemctl enable raind
# 移除开机自启
$ sudo systemctl disable raind
# 查看状态和部分日志
$ sudo systemctl status raind
```

### 配置文件

```shell
# 注意:首次启动时会自动创建默认的配置文件并退出
# 编辑配置文件
$ sudo vim /usr/local/etc/rain.conf
# 写入自己的配置后重新启动即可

# 指定服务器地址(必须指定)
# IPv4地址和域名是受支持的
ServAddr = 127.0.0.1

# 指定服务器端口(必须指定)
ServPort = 1234

# 客户端身份鉴别,此选项仅在严格模式有效,用于解释客户端如何和服务器连接
# 0: 不加载任何安全文件,客户端允许以ssl连接到任何特定服务器,即使服务器证书有误
# 1: 仅加载CA证书,这使得客户端可以鉴别服务器身份,客户端可以信任服务器的合法性
# 2: 加载CA证书,客户端用户证书,客户端私密钥,仅当服务器要求验证客户端的合法性时指定,当然服务器不要求时指定了也无影响
SSLMode = 0

# 指定CA根证书(用以验证服务器身份)
CAfile = /home/pi/keys/ca.crt

# 指定用户证书(仅当 CheckMe = true)
UCert = /home/pi/keys/client.crt

# 指定用户私秘钥(仅当 CheckMe = true)
UKey = /home/pi/keys/client.key

# 数据采样间隔(单位:秒)(默认值:10)
Interval = 10 

# 失败重连等待时间(单位:秒)(默认值:42)
FRecTime = 42

# 失败重连尝试次数(单位:秒)(默认值:7)
FRecAtps = 7
```

### 常见问题

```text
编译时找不到ssl.h,wringPi.h
    这是缺少了相关库,安装openssl-dev库,
    另外如果你使用的是树莓派,请安装wringPi库,否则请开启模拟器模式

0. 程序运行没结果(直接返回命令行)
    a.推荐查看日志(看下面第三条)
    b.程序没有运行起来(看下面第一条)
    c.变成了守护进程,如果不想进入守护进程,可以进入build/bin目录手动执行客户端程序且不要加上"-d"参数
    
1. 程序跑不起来:
    a.检测配置文件是否正确配置,其中服务器端口和地址是必须指定的
    b.查看pid文件 /var/run/raind.pid 是否存在
        你看到了pid文件当且仅当:
            I.  已经有相同客户端程序在运行,使用 ps aux | grep raind 命令找出它的PID,并使用 sudo kill -INT <PID>即可
            II. 没有程序运行(一般是上一次运行时被暴力结束了,因此遗留文件未删除),这时直接删掉文件即可; 
                pid文件在程序启动时自动创建,终止时自动删除.因此一般情况下,您无需手动操作
        
2.  System has not been booted with systemd as init system (PID 1). Can't operate.
    即 systemd 用不了(sudo systemctl start/stop/enable/disable/status... raind)
    
    造成这种原因可能是您的程序运行在没有完整独立内核和完全的系统调用的Linux虚拟化平台上,比如 WSL1 (Windows Subsystem For Linux Version.1)
    当然也可以自行百度 "WSL开启systemctl"
    否则在这种情况下,直接手动执行可执行文件即可,您没有必要 make install 了(根本无法使用systemd)
    
3. 不清楚程序现在是什么状态
    我们会将关键步骤和出错信息写入系统日志(设施:daemon)
    正常情况下,直接打开/var/log/daemon.log即可看见,如果没有该文件,您可以:
        a. sudo vim /etc/rsyslog.conf
        b. 添加:
                    "daemon.*                        -/var/log/daemon.log"
           后保存并退出
        c. 重启系统或者 sudo systemctl restart rsyslog 即可
        d. 看不懂自行百度 "rsyslog日志输出"相关内容
        
4. 跨设备通讯时两端都成功运行起来了,但是无法建立连接
    a. 保持端口一致,建议使用数值大一点的端口,避免端口冲突
    b. 查看目标网络是否是可达的
    c. 使用云服务器的童鞋记得在安全组面板里面放行端口
    
5. 最后来个小提示:
    如果长时间没有成功连接服务器,超出你设置的最大重连次数后程序就退出了
```

## 工程结构

### 目标依赖关系:

*标识:弃用

```text
    依赖(target) <- 被依赖
    conf        <- arr filed linklist
    filed       <- log
    linklist    <- types
    load_clean  <- log raspi_drive
    rssl        <- crypto(external) log rssl
    client_run  <- conf log load_clean socket_fd raspi_drive types
    raspi_drive <- log wiringPi(external)
```

### 库函数清单:

```text
    arr:
        isNotes,isEmptyL,isContainC
        notASCII,rmNextL,rmCharacter
        subString,upperConversion,lowerConversion

    conf:
        readConf,defaultConfClnt,checkConf
        checkRead

   *file:
        file_size,readopen,writeopen
        rwopen,newopen,fcopyfile
    filed:
        setFiledLogLevel,fileSize,readOpen
        writeOpen,rwOpen,newOpen
        copyFile
        
   *msg:
        errors,warnings,success
        perr
    msgd:
        log_prefix,perr

    mylink:
        InitLinkList,CreateLinkList,DestroyLinkList
        LengthOfLinkList,InsertIntoLinkList,GetElemFromLinkList
        DeleteElemAtLinkList,DisplayLinkList,ListToArry
        
    myprocess:
        daemonize,exitCleanupClnt,resetClnt
        checkPidFileClnt,checkRootPermission,sigRegisterClnt
        runTimeArgsClnt

   *mysock:
        NameToHost,creatServSock,connectServ
        acceptClnt,sockReuseAddr,sockNagle
    mysockd:
        setFiledLogLevel,NameToHost,creatServSock
        connectServ,acceptClnt,sockReuseAddr
        sockNagle,setSockFlag,checkFd
        
    myssl:
        initSSL,loadCA,loadCert
        loadKey,checkKey,showPeerCert
        showSelfCert,SSL_fd
        
    mythread:
        confToVarClnt,tryConnect,checkMonit
        sendData

    raspi:
        initPi,read_cpu_temp,turn_on_led
        turn_off_led,flash_led,disMeasure
        TEST_IN,ReadSensorData,screen

    *sundry:
        procbar,procbar_f,isDebugger

    types:
        setElem,delElem,catElem
        catElems
```