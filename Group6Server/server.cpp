#include "server.h"

Server::Server(QObject *parent):
    QTcpServer(parent)
{

}

//! PUBLIC
bool Server::StartServer(){
    //默认使用host即“127.0.0.1”
    //调用listen函数，在指定的端口号中进行任意监听 ps：Any参数表示任意IPv4地址0.0.0.0
    if(!this->listen(QHostAddress::Any,port)){
        qDebug() << "无法启动服务端！";
        return false;
    }
    else{
        qDebug() << "server 监听中...";
        return true;
    }
}
/** 初始化端口号*/
void Server::setPort(quint16 port){
    this->port = port;
    return;
}

/** 服务器端退出（所有人下线）*/
void Server::close(){
    //使用foreach遍历，关闭所有连接到server的线程
    foreach(ClientThread* client,clientThreadList){
        client->close();
    }
    //关闭server
    QTcpServer::close();
}
/** 将消息发送给所有人*/
void Server::sendTextToAll(QString text,ClientThread* except){
    //用于暂存要发送的数据
    QByteArray block;
    //使用数据流写入数据 只写（设置为ReadWrite则为读写）
    QDataStream out(&block,QIODevice::WriteOnly);
    //设置数据流的版本，客户端server与服务端Client版本需要相同
    out.setVersion(QDataStream::Qt_5_8);

    //设置初始值为0，设置长度
    out << (quint32)0 << text;

    //回到字节流的起始位置
    out.device()->seek(0);
    //重置字节流长度
    out << (quint32)(block.size() - sizeof(quint32));
    qDebug() << "block.size() = " << block.size();//调试用（打印出block的长度）

    qint64 x = 0;
    //遍历线程列表，如果是空，或者
    foreach(ClientThread* eachClient,clientThreadList){
        if(except != NULL && eachClient == except)
            continue;
        x = 0;
        while(x < block.size()){
            //向套接字缓存中写入数据
            qint64 y = eachClient->getTcpSocket()->write(block);
            x+=y;
            //缓冲池在首次连接的时候没有数据，在首次连接成功时候打印出发送者
            qDebug() << eachClient->getUsername()<< "/sent" << x ;//调试用（输出发送者）
        }
        qDebug() << "-----";
    }
}

/**
 * 私聊功能
 * 参数为String类型字符串，与目标线程 target
 * 注释同上
 */
void Server::sendTextToOne(QString text,ClientThread* target){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);

    out << (quint32)0 << text;

    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));
    qDebug() << "block.size() = " << block.size();

    qint64 x = 0;
    while(x < block.size()){
        qint64 y = target->getTcpSocket()->write(block);
        x+=y;

        qDebug() << target->getUsername()<< " /sent " << x ;
        qDebug() << "-----";
    }
}
/**
 * 群发文件（每个人都可以选择接收）
 * 新添功能,暂不添加注释
 */
void Server::sendDataFileToAll(QString text,QByteArray dataOfFile,ClientThread* except){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);

    out << (quint32)0 << text;
    block.append(dataOfFile);

    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));
    qDebug() << "block.size() = " << block.size();

    qint64 x = 0;
    foreach(ClientThread* eachClient,clientThreadList){
        if(except != NULL && eachClient == except)
            continue;
        x = 0;
        while(x < block.size()){
            qint64 y = eachClient->getTcpSocket()->write(block);
            x+=y;
            qDebug() << eachClient->getUsername()<< "/sent file: " << x ;
        }
        qDebug() << "-----";
    }
}

/**
 * 私发文件
 * 暂时不添加详细注释
 */
void Server::sendDataFileToOne(QString text,QByteArray dataOfFile,ClientThread* target){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);

    out << (quint32)0 << text;
    block.append(dataOfFile);

    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));
    qDebug() << "block.size() = " << block.size();

    qint64 x = 0;
    while(x < block.size()){
        qint64 y = target->getTcpSocket()->write(block);
        x+=y;
        qDebug() << target->getUsername()<< "/sent file: " << x ;
    }
    qDebug() << "-----";
}
/** 踢人功能*/
void Server::kickFromServer(int socketDescriptor){
    //QlinkedList为qt中的一种容器（类似于java中的集合list<>）
    //iterator为建立在指针基础上的迭代器
    QLinkedList<ClientThread*>::iterator iter;
    //找到list中的socket文件描述符然后将其关闭
    //从列表首 开始 一直循环 一直到列表的end，（++iter表示指向下一个）
    for(iter = clientThreadList.begin();iter != clientThreadList.end(); ++iter){
        //当找到对应的socket文件描述符时候将其关闭
        if((*iter)->getSocketDescriptor() == socketDescriptor){
            (*iter)->close();
            return;
        }
    }

}

//! PUBLIC


//! SLOTS
void Server::on_client_connected(ClientThread* clientThread){
    clientThreadList.push_back(clientThread);
//    qDebug() << "added";//调试用


}

void Server::on_client_disconnected(ClientThread* clientThread){
    //从列表删除clientThread
    deleteClientFromList(clientThread);
    //获取用户名列表控件
    QStringList usernameList;
    //遍历用户名，添加到userlist中（使用append时候会自动换行，而使用insertPlainText则不会）
    foreach(QString username,users.values()){
        usernameList.append(username);
    }

    //一个人退出时通知所有人，并且发送到用户列表
//    foreach(ClientThread* eachClient,clientThreadList){
//        eachClient->getTcpSocket()->write(QString("/server:" + clientThread->getUsername() + " has left.\n").toUtf8());
//        eachClient->getTcpSocket()->write(QString("/users:" + usernameList.join(",") +"\n").toUtf8());
//    }
    //服务端左右两侧列表，以及输入其中的数据格式
    QString messageUserLeft = "/server:" + clientThread->getUsername() + " has left.\n";
    QString messageUserNewList= "/users:" + usernameList.join(",") +"\n";
    sendTextToAll(messageUserLeft);
    sendTextToAll(messageUserNewList);

}
/**
 * 用户名查重机制
 * 如果名字重复  会有消息
 */
void Server::on_client_usernameChanged(QString uname){
    ClientThread* client = (ClientThread*)sender();
    QStringList nameList = users.values();
    //如果用户名字重复，会自动在名字后面加上下划线+数字比如  "陈旗开_1"
    //并且告诉用户名字已经修改（调用sendTextToOne()）
    if(nameList.contains(uname)){
        int count = 1;
        //正则匹配 检查每一个重复的名字
        QRegularExpression dupName("^"+uname+"_\\d+$");
        QRegularExpressionMatch matchName;
        //遍历
        foreach(QString name,nameList){
            if(dupName.match(name).hasMatch()){
                count++;
            }
        }

        uname = uname + "_" + QString::number(count);
//        client->getTcpSocket()->write(QString("/nameDup:"+uname+"\n").toUtf8());
        QString messageNameDup = "/nameDup:" + uname + "\n";
        sendTextToOne(messageNameDup,client);

        client->setUsername(uname);
    }
    users[client] = uname;

    QStringList usernameList;
    foreach(QString username,users.values()){
        usernameList.append(username);
    }

//    //其他人上线通知
//    //重新发送到用户列表（复用代码块）
//    foreach(ClientThread* eachClient,clientThreadList){
//        eachClient->getTcpSocket()->write(QString("/server:" + uname + " has joined.\n").toUtf8());
//        eachClient->getTcpSocket()->write(QString("/users:" + usernameList.join(",") +"\n").toUtf8());
//    }


    //设定信息字符串格式
    QString messageUserJoin = "/server:" + uname + " has joined.\n";
    QString messageUserNewList = "/users:" + usernameList.join(",")+"\n";
    sendTextToAll(messageUserJoin);
    sendTextToAll(messageUserNewList);

    emit clientUsernameChanged(client->getSocketDescriptor(),uname);
}

/**
 * 用户群聊信息 发送处理函数
 * @param 传过来的用户名和文本信息
 * 将文本处理并且加上时间，最后调用sendTextToAll（）转发
 */
void Server::on_client_textSend(QString uname,QString text){
    //实体化发送者的线程
    ClientThread* sender_client = (ClientThread*)sender();
    //获取当前系统时间
    QString currentTime = QTime::currentTime().toString("H:m A/");

    QString message = "/text:"+currentTime + uname + " : " + text + "\n";
//    foreach(ClientThread* client,clientThreadList){
//        if(sender_client != client){
//            client->getTcpSocket()->write(message.toUtf8());
//        }
//    }
    sendTextToAll(message,sender_client);

}
/**
 * 私发消息 发送处理函数
 * @param 获取传过来的发送者名，接收者名，和文本
 * 将文本格式处理，之后调用sendTextToOne（）函数转发
 */
void Server::on_client_privateTextSend(QString uname,QString receiverName,QString text){
    QString currentTime = QTime::currentTime().toString("H:mm A/");
    QString messagePm = "/pm:"+currentTime+uname+" : "+text+"\n";
    foreach(ClientThread* client,clientThreadList){
        if(client->getUsername() == receiverName){
            sendTextToOne(messagePm,client);
            return;
        }
    }
}

/** 发送文件（暂时不写注释）*/
void Server::on_client_fileSend(QString uname,QString filename,QByteArray dataOfFile){
    ClientThread* client = (ClientThread*)sender();
    QString currentTime = QTime::currentTime().toString("H:mm A/");
    QString command = "/fileAll:"+currentTime + uname + " : " + filename + "\n";
    this->sendDataFileToAll(command,dataOfFile,client);
}
/** 私发文件（暂时不写注释）*/
void Server::on_client_privateFileSend(QString uname,QString receiverName, QString filename, QByteArray dataOfFile){
    QString currentTime = QTime::currentTime().toString("H:mm A/");
    QString command = "/filePrivate:"+currentTime + uname + " : " + filename + "\n";
    foreach(ClientThread* client,clientThreadList){
        if(client->getUsername() == receiverName){
            sendDataFileToOne(command,dataOfFile,client);
            return;
        }
    }
}

//! SLOTS


//! PROTECTED、
/**
 * QTcpServer中的虚函数与socketDescriptor配套使用
 * 将每一个连接过来的socket文件描述符添加到列表中，该方法为必须覆盖的方法之一
 */
void Server::incomingConnection(qintptr socketDescriptor){
    qDebug() << socketDescriptor << " 连接中...";
    QTcpSocket *socket = new QTcpSocket();
    ClientThread *cliThread = new ClientThread(socketDescriptor,socket,this);

    //为连接进来的线程绑定事件
    connect(cliThread, SIGNAL(finished()),cliThread,SLOT(deleteLater()));
    connect(cliThread, SIGNAL(connected(ClientThread*)),this,SIGNAL(connected(ClientThread*)));
    connect(cliThread, SIGNAL(connected(ClientThread*)),this,SLOT(on_client_connected(ClientThread*)));
    connect(cliThread, SIGNAL(clientDisconnected(ClientThread*)),this,SIGNAL(clientDisconnected(ClientThread*)));
    connect(cliThread, SIGNAL(clientDisconnected(ClientThread*)),this,SLOT(on_client_disconnected(ClientThread*)));
    connect(cliThread, SIGNAL(usernameChanged(QString)),this,SLOT(on_client_usernameChanged(QString)));
    connect(cliThread, SIGNAL(textSend(QString,QString)),this,SLOT(on_client_textSend(QString,QString)));
    connect(cliThread, SIGNAL(privateTextSend(QString,QString,QString)),this,SLOT(on_client_privateTextSend(QString,QString,QString)));
    connect(cliThread, SIGNAL(fileSend(QString,QString,QByteArray)),this,SLOT(on_client_fileSend(QString,QString,QByteArray)));
    connect(cliThread, SIGNAL(privateFileSend(QString,QString,QString,QByteArray)),this,SLOT(on_client_privateFileSend(QString,QString,QString,QByteArray)));


    socket->moveToThread(cliThread);
    cliThread->start(); //启用线程
}

//! PROTECTED


/**
 *
 * 从Client列表删除一个数据
 * PRIVATE
 */
void Server::deleteClientFromList(ClientThread* client){
    // 在 on_client_disconnected 中调用
    clientThreadList.removeOne(client);
    users.remove(client);

    qDebug() << "删除.";//用于调试
}

//! PRIVATE
