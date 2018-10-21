#                                                       EVP RPC说明

这个小组件主要细分为2个模块：

proxy:  负责网络通讯，接受网络包后按自定义协议解析网络包，塞入worker共享内存读队列供worker消费，同 

​             时从worker共享内存写队列读包，转发到网络客户端。

worker: 负责业务逻辑处理，消费worker共享内存读队列消息，根据业务逻辑处理请求，将应答写入worker共享

​               内存写队列。

![QQ20181021204650](C:\Users\tyreezhang\Desktop\QQ20181021204650.png)



