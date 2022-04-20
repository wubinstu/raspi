# 提示:生成证书时需要提供一些信息,可以使用工具faker生成测试数据
# 本文件夹中生成了一些案例证书,其中:
# ca.key密码:0000
# server.key密码:123456
# client.key密码:654321


# 生成CA证书(需要为ca.key设置密码如0000,然后会询问输入国家地区组织等信息)
$ openssl req -new -x509 -keyout ca.key -out ca.crt


# 生成服务器私钥(需要为server.key设置密码,如123456)
$ openssl genrsa -des3 -out server.key 4096

# 生成证书请求(需要输入方才的密码,然后会询问输入国家地区组织等信息)
$ openssl req -new -key server.key -out server.csr


# 查找默认的openssl.cnf文件
$ sudo find / -name openssl.cnf
# 比如 "/etc/ssl/openssl.cnf"

# 创建一些目录和文件,名称在方才的openssl.cnf文件有提示(即下面的demoCA,newcerts,index.txt,serial不一定是固定的)
$ mkdir -p demoCA/newcerts
$ touch demoCA/index.txt && echo 01 > demoCA/serial

# 生成服务器证书,如果出错就仔细检查上面的配置是否正确设置
$ openssl ca -in server.csr -out server.crt -cert ca.crt -keyfile ca.key -config /etc/ssl/openssl.cnf



# 客户端(和服务端几乎相同操作)
# 生成客户端私钥(密码,如654321)
$ openssl genrsa -des3 -out client.key 4096

# 生成客户端证书请求
$ openssl req -new -key client.key -out client.csr

# 生成客户端证书
$ openssl ca -in client.csr -out client.crt -cert ca.crt -keyfile ca.key