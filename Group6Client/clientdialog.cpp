#include "clientdialog.h"
#include "ui_dialog.h"
/**
 * 初始化界面（用户登录界面）
 * @date 2018年12月13日
 */
ClientDialog::ClientDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClientDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("欢迎来到网工三班第六组的聊天室～☺");
    //设定当前page
    ui->stackedWidget->setCurrentWidget(ui->Page_1);

    //初始化套接字
    tcpSocket = new QTcpSocket(this);
    //初始化定时器 与 时钟
    myTimer = new QTimer(this);
    myLCDNumber = new QLCDNumber(this);
    //設定位置大小
    myLCDNumber->setGeometry(300,30,150,30);
    //设置时钟位置
    myLCDNumber->setDigitCount(8);
    //以1000毫秒为周期启动定时器
    myTimer->start(1000);
    /*连接信号*/
    connect(tcpSocket,SIGNAL(connected()),this,SLOT(connected()));
    //必要方法之一，读取数据用，连接到信号槽
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(displayError(QAbstractSocket::SocketError)));
    connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(disconnected()));
    //连接 时钟信号
    connect(myTimer,SIGNAL(timeout()),this,SLOT(showTime()));


}

/** 时钟*/
void ClientDialog::showTime()
{
    QTime time = QTime::currentTime();
    QString text=time.toString("hh:mm:ss"); //时间格式
    myLCDNumber->display(text);
}
/**
 * 关闭窗口
 */
ClientDialog::~ClientDialog()
{
    foreach(PrivateDialog* pmDialog,privateChatList.values()){
        pmDialog->close();
    }
    delete ui;
}

/**
 * 发送message到服务端
 * @param text
 */
void ClientDialog::sendTextToServer(QString text){
    QByteArray block;
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);

    out << (quint32)0 << text;

    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));

    qint64 x = 0;
    while(x < block.size()){
        qint64 y = tcpSocket->write(block);
        x+=y;
        qDebug() << x;
    }
    qDebug() << "-----";

}
/**
 * 发送文件
 */
void ClientDialog::sendFileToAll(QString filePath,QString filename){
    //通过fileTcpSocket发送文件

    //发送的数据存入block中
    QByteArray block;
    QFile file(filePath);
    //打开文件，定义错误提醒
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"","无法打开文件！");
        return;
    }
    //打开文件流，以写的方式打开
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);
    QString command = "/fileAll:"+filename+"\n";
    //block初始值为0 并且设定阻塞待定命令
    out << (quint32)0 << command;
    //将数据读取到QByteArray
    QByteArray dataOfFile = file.readAll();
    //关闭文件
    file.close();
    //放入block
    block.append(dataOfFile);
    //定义游标初始位置
    out.device()->seek(0);
    //将block的大小设定为实际大小
    out << (quint32)(block.size() - sizeof(quint32));

    qint64 x = 0;
    while(x < block.size()){
        qint64 y = tcpSocket->write(block);
        x+=y;
        qDebug() << "sent file: "<< x << "of " << block.size();
    }
    qDebug() << "-----";
}

/** 接收文件*/
void ClientDialog::onFileReceived(QString senderName,QString filename,QByteArray dataOfFile){
    QMessageBox::StandardButtons reply;
    reply = QMessageBox::question(this,tr("文件已经接收"),
                             tr("%1 send you a file \"%2\"\nDo you want to save a file?")
                                  .arg(senderName,filename));
    if(reply == QMessageBox::No)
        return;
    QString homePath = QDir::homePath();
    QString filePath = QFileDialog::getExistingDirectory(this,
                                                    tr("Open Directory"),
                                                    homePath,
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    QFile outFile(filePath + "/" +filename);
    if(!outFile.open(QIODevice::WriteOnly)){
        QMessageBox::warning(this,"","错误: 无法打开文件.");
        return;
    }
    outFile.write(dataOfFile);

    outFile.close();

}

//! PUBLIC

//! PUBLIC SLOTS
/**
 * 聊天室主界面
 * @brief 连接成功之后跳转到该界面,
 */
void ClientDialog::connected(){
    ui->stackedWidget->setCurrentWidget(ui->Page_2);
    ui->lineEdit_ServerIP->setText(this->serverIP + "::" + QString::number(this->port));
    // 将用户名发送到server
//    tcpSocket->write(QString("/username:" + username + "\n").toUtf8());
    //发送的名字格式
    QString messageUsername = "/username:" + username +"\n";
    this->sendTextToServer(messageUsername);


}

/** */
void ClientDialog::readyRead(){
    QRegularExpression regex_text("^/text:(.*)/(.*) : (.*)\n$");
    QRegularExpression regex_server("^/server:(.*)\n$");
    QRegularExpression regex_users("^/users:(.*)\n$");
    QRegularExpression regex_private("^/pm:(.*)/(.*) : (.*)\n$");
    QRegularExpression regex_error("^/nameDup:(.*)\n$");
    QRegularExpression regex_fileAll("^/fileAll:(.*)/(.*) : (.*)\n$");
    QRegularExpression regex_filePrivate("^/filePrivate:(.*)/(.*) : (.*)\n$");
    QRegularExpressionMatch match;

    qDebug() << "1.canReadLine = "<< tcpSocket->canReadLine();
    qDebug() << "1.blockSize = " << blockSize;
    while(tcpSocket->canReadLine()){
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_8);

    if (blockSize == 0){
        if(tcpSocket->bytesAvailable() < sizeof(quint32))
            return;
        in >> blockSize;
        qDebug() << "blockSize "<< blockSize;
    }
    if(tcpSocket->bytesAvailable() < blockSize){
        qDebug() << "byteAval "<< tcpSocket->bytesAvailable();
        return;
    }

    QString data;
    in >> data;

    qDebug() << data;
    //match for text message
    match = regex_text.match(data);
    if(match.hasMatch()){
        //update chat display
        ui->textEdit_ChatDisplay->append(tr("<b>%1 <font color = \"LightGrey\">[%2]</font>: </b> %3")
                                         .arg(match.captured(2),match.captured(1),match.captured(3)));
    }
    //match for server message
    match = regex_server.match(data);
    if(match.hasMatch()){
        //update chat display
        ui->textEdit_ChatDisplay->append(
                    "<font color=\"gray\">" +match.captured(1)+"</font><br>");
    }
    //match for users list update
    match = regex_users.match(data);
    if(match.hasMatch()){
        //用户列表添加一个信息（包括图像）
        QStringList users = match.captured(1).split(",");
        ui->listWidget_OnlineUsers->clear();
        foreach(QString user,users){
            new QListWidgetItem(QPixmap("://images/User-icon.png"), user, ui->listWidget_OnlineUsers);
            if(user == this->username){
                QList<QListWidgetItem*> list = ui->listWidget_OnlineUsers->findItems(user,Qt::MatchExactly);
                foreach(QListWidgetItem* item,list){
                    item->setTextColor(Qt::darkRed);
                }
            }

        }
    }
    //match for private message
    match = regex_private.match(data);
    if(match.hasMatch()){
        QString senderName = match.captured(2);
        QString time = match.captured(1);
        QString text = match.captured(3);
        //if already open just append it
        if(!privateChatList.contains(senderName)){
            privateChatList[senderName] = new PrivateDialog(this->username,senderName,this);
            connect(privateChatList[senderName],SIGNAL(finished(int)),this,SLOT(privateFinished(int)));
            connect(privateChatList[senderName],SIGNAL(messageSent(QString,QString)),this,SLOT(privateMessageSent(QString,QString)));
            connect(privateChatList[senderName],SIGNAL(fileSend(QString,QString,QString)),this,SLOT(privateFileSent(QString,QString,QString)));
            privateChatList[senderName]->show();

        }
        privateChatList[senderName]->addNewMessage(time,text);
    }
    //match for server error nampDuplicate
    match = regex_error.match(data);
    if(match.hasMatch()){

        QString new_name = match.captured(1);
        QMessageBox::information(this,tr("Information"),"Your name \""+this->username+"\" is duplicated.\n"+
                                                        "Your name has changed to \""+new_name+"\".");
        this->username = new_name;
    }
    //match for file received
    match = regex_fileAll.match(data);
    if(match.hasMatch()){
        QString time = match.captured(1);
        QString senderName = match.captured(2);
        QString filename = match.captured(3);
        QByteArray dataOfFile = tcpSocket->readAll();
        ui->textEdit_ChatDisplay->append(tr("<b>%1 <font color = \"LightGrey\">[%2]</font> sent a file <font color = \"DarkSlateBlue\">\"%3\"</font></b>")
                                         .arg(senderName,time,filename));
        onFileReceived(senderName,filename,dataOfFile);
    }
    //match for private file received
    match = regex_filePrivate.match(data);
    if(match.hasMatch()){
        QString time = match.captured(1);
        QString senderName = match.captured(2);
        QString filename = match.captured(3);
        QByteArray dataOfFile = tcpSocket->readAll();
        //if already open just append it
        if(!privateChatList.contains(senderName)){
            privateChatList[senderName] = new PrivateDialog(this->username,senderName,this);
            connect(privateChatList[senderName],SIGNAL(finished(int)),this,SLOT(privateFinished(int)));
            connect(privateChatList[senderName],SIGNAL(messageSent(QString,QString)),this,SLOT(privateMessageSent(QString,QString)));
            connect(privateChatList[senderName],SIGNAL(fileSend(QString,QString,QString)),this,SLOT(privateFileSent(QString,QString,QString)));
            privateChatList[senderName]->show();
        }
        QString text = "sent a file <font color = \"DarkSlateGray\"><b>\"" + filename + "\"</b></font>";
        privateChatList[senderName]->addNewMessage(time,text);
        onFileReceived(senderName,filename,dataOfFile);
    }
    blockSize = 0;
    qDebug() << "2.canReadLine = "<< tcpSocket->canReadLine();

    }
}

/** */
void ClientDialog::disconnected(){
    ui->stackedWidget->setCurrentWidget(ui->Page_1);
    ui->textEdit_ChatDisplay->clear();
    ui->lineEdit_ServerIPAddr->setFocus(Qt::TabFocusReason);
    QMessageBox::information(this,tr("Disconnected"),tr("You are disconnected from server."));

}

/** */
void ClientDialog::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Socket Error"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Socket Error"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Socket Error"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }
}

/** */
void ClientDialog::privateFinished(int result){
    PrivateDialog *privateDialog = (PrivateDialog*)sender();
    privateChatList.remove(privateDialog->getUsername());
    qDebug() << "removed";
}

/** */
void ClientDialog::privateMessageSent(QString username,QString text){
//    tcpSocket->write(QString("/pm:"+username+ " : "+text+"\n").toUtf8());
    QString message = "/pm:"+username+" : "+text + "\n";
    this->sendTextToServer(message);

}

/** */
void ClientDialog::privateFileSent(QString username,QString filePath,QString filename){
    //block to be sent
    QByteArray block;
    QFile file(filePath);
    //open file and check error
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"","Can't open file for transfer");
        return;
    }
    //datastream
    QDataStream out(&block,QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_8);
    QString command = "/filePrivate:"+username+" : "+filename+"\n";
    //initial block size to 0 and input aspecific command to block
    out << (quint32)0 << command;
    //read all data of file to QByteArray
    QByteArray dataOfFile = file.readAll();
    //close the file
    file.close();
    //add it to block
    block.append(dataOfFile);
    //seek to the beginning
    out.device()->seek(0);
    //set the block size to actual block size - the size of stored block size
    out << (quint32)(block.size() - sizeof(quint32));

    qint64 x = 0;
    while(x < block.size()){
        qint64 y = tcpSocket->write(block);
        x+=y;
        qDebug() << "sent file: "<< x << "of " << block.size();
    }
    qDebug() << "-----";
}

//! PUBLIC SLOTS
/** */
void ClientDialog::on_pushButton_Login_clicked()
{
    QString serverIP = ui->lineEdit_ServerIPAddr->text();
    QString port = ui->lineEdit_port->text();
    QString username = ui->lineEdit_username->text();
    QIntValidator portValidator(1,65535);
    int pos = 0;

    if(portValidator.validate(port,pos) == QIntValidator::Invalid || port.isEmpty()){
        QMessageBox::warning(this,tr("Input Error"),tr("Invalid port.\n(Must be 1 - 65535)"));
        return;
    }
    else if(username.isEmpty()){
        QMessageBox::warning(this,tr("Input Error"),tr("Invalid username.\n(Must contain some characters.)"));
        return;
    }

    QHostAddress servAddr(serverIP);
    this->serverIP = serverIP;
    this->port = port.toUInt();
    this->username = username;

    tcpSocket->connectToHost(servAddr,this->port);
    tcpSocket->waitForConnected(30000); //wait 30 seconds for socket to be connected to host successfully
                                        //if not connected, emit error.
}

/** */
void ClientDialog::on_pushButton_Disconnect_clicked()
{
    foreach(PrivateDialog *pmDialog,privateChatList.values()){
        pmDialog->close();
    }
    tcpSocket->close();
}

/** 文本框敲击回车 获取输入的文本，并且执sendToServer（），将消息发送给server*/
void ClientDialog::on_lineEdit_ChatInput_returnPressed()
{
    QString text = ui->lineEdit_ChatInput->text().trimmed();

    if(text.isEmpty())
        return;

    //服务器识别带有"/text"的文本
//    tcpSocket->write(QString("/text:" + text +"\n").toUtf8());
    QString message = "/text:"+text +"\n";
    this->sendTextToServer(message);

    //get current time string
    QString currentTime = QTime::currentTime().toString("H:m A");
    //add to chat display
    ui->textEdit_ChatDisplay->append("<font color=\"DarkRed\"><b>"+username+" ["+currentTime+"]"+" :</b> " + text+"</font>");

    //clear chat input
    ui->lineEdit_ChatInput->clear();
}

/** 私聊按钮点击事件*/
void ClientDialog::on_pushButton_PrivateChat_clicked()
{
    //解决没有用户点击崩溃，添加错误判断
    if(!ui->listWidget_OnlineUsers->count()){
        return;
    }
    if(ui->listWidget_OnlineUsers->selectedItems().count() == 0){
        return;
    }

    //从选中的item中获取数据
    QString username = ui->listWidget_OnlineUsers->currentItem()->text();
    //不会重复打开聊天框
    if(privateChatList.contains(username)){
        return;
    }

    privateChatList[username] = new PrivateDialog(this->username,username,this);
    connect(privateChatList[username],SIGNAL(finished(int)),this,SLOT(privateFinished(int)));
    connect(privateChatList[username],SIGNAL(messageSent(QString,QString)),this,SLOT(privateMessageSent(QString,QString)));
    connect(privateChatList[username],SIGNAL(fileSend(QString,QString,QString)),this,SLOT(privateFileSent(QString,QString,QString)));
    privateChatList[username]->show();

}

/** */
void ClientDialog::on_pushButton_ChooseFile_clicked()
{
    //set home directory to user home path
    QString homePath = QDir::homePath();
    //open file dialog and choose a file and get the file path
    QString filePath = QFileDialog::getOpenFileName(this,tr("Open a file"),homePath,tr("All files (*.*)"),NULL,QFileDialog::DontResolveSymlinks);
    //get the filename
    QString filename = filePath.section("/",-1);
    //do nothing if no file selected
    if(filename.isEmpty())
        return;

    //confirmation
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,"Send a file",tr("Do you want to send \"%1\"?").arg(filename));

    if (reply == QMessageBox::No )
        return;
    else{
        sendFileToAll(filePath,filename);
        //get current time string
        QString currentTime = QTime::currentTime().toString("H:mm A");
        //add to chat display
        ui->textEdit_ChatDisplay->append("<font color=\"DarkRed\"><b>"+username+" ["+currentTime+"] send a file \""+filename+"\"</b></font>");
    }


}
