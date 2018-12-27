# Linux环境 + QT Creator + C++语言 实现聊天室

期末大作业，嵌入式综合设计。
基于tcp/ip协议的网路聊天程序，分为服务端与客户端。
首先运行服务端，打开端口监听，之后运行客户端连接。
- 截图：
1. server端
![image.png](https://upload-images.jianshu.io/upload_images/13139591-e184ca2a05859efc.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

2. client端
![image.png](https://upload-images.jianshu.io/upload_images/13139591-6791cf909d279d53.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

- 程序名称：Group6ChatRoom
- 程序功能：
| 功能名称 | 完成情况 | 具体内容 |
| ------ | ------ | ------ |
| 实现两端通讯 | 已完成 | server开启监听等待到来的client并创建线程处理 |
| 私聊功能 | 有bug | 聊天会弹出dialog（再次打开会话时候有bug） |
| 下线 | 已完成 | 主动下线和被动下线 |
| 踢人下线功能 | 已完成 | 关闭连接进来的client（其对应的线程和socket描述符） |
| 实现日志存入系统文件 | 已完成 | Qdebug信息自动保存 |
| 聊天记录保存 | 已完成 | 聊天记录在程序目录下client文件夹存储 |
| 私聊记录保存 | 有bug | 私聊dialog中敲击回车会自动保存两端记录 |
| 添加实时钟表 | 已完成 | 两端界面都会显示 |
| 实现文件收发 | 进行中 | 私聊发送文件 |
| 用户列表使用随机头像 | 进行中 | 为每个用户分配对应头像 |
| 自定义背景 | 进行中 | 用户可以选择背景自己修改 |
| 读取聊天记录 | 进行中 | 用户从本地读取自己的聊天记录 |

背景图片来自喜爱的画师 [WLOP](https://www.patreon.com/wlop)    
并感谢提供部分代码参考的大佬[FFEVER](https://github.com/FFEVER)
