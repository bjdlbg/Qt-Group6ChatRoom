#include "serverdialog.h"
#include "ui_dialog.h"
/**
 *
 * @brief 初始化服务器界面，打开窗口
 * @date 2018年12月16日
 */
ServerDialog::ServerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerDialog)
{
    ui->setupUi(this);
    //给窗口设置标题
    this->setWindowTitle("欢迎来到网工三班第六组的聊天室～:)");
    //设定当前的页面（page1为启动服务的界面）
    ui->stackedWidget->setCurrentWidget(ui->page_1);
    //端口输入验证（如果失败，调出提醒dialog）
    validator_port = new QIntValidator(1,65535,ui->lineedit_port);
    //初始化定时器 与 时钟
    myTimer = new QTimer(this);
    myLCDNumber = new QLCDNumber(this);
    //設定位置大小
    myLCDNumber->setGeometry(300,30,150,30);
    //设置时钟位置
    myLCDNumber->setDigitCount(8);
    //以1000毫秒为周期启动定时器
    myTimer->start(1000);
    showTime();
    //连接信号槽
    //界面跳转时候
    connect(ui->stackedWidget,SIGNAL(currentChanged(int)),this->validator_port,SLOT(deleteLater()));
    //连接 客户端线程接入信号
    connect(&server,SIGNAL(connected(ClientThread*)),this,SLOT(on_clientThread_connected(ClientThread*)));
    //连接 ～～～退出信号
    connect(&server,SIGNAL(clientDisconnected(ClientThread*)),this,SLOT(on_clientThread_disconnected(ClientThread*)));
    //连接 用户名变更提醒信号
    connect(&server,SIGNAL(clientUsernameChanged(int,QString)),this,SLOT(on_clientThread_usernameChanged(int,QString)));
    //连接 时钟信号
    connect(myTimer,SIGNAL(timeout()),this,SLOT(showTime()));

}

/** 时钟*/
void ServerDialog::showTime()
{
    QTime time = QTime::currentTime();
    QString text=time.toString("hh:mm:ss"); //时间格式
    myLCDNumber->display(text);
}

/**
 * @brief 关闭监听
 * ~ServerDialog()为QTcpServer类中必须覆盖方法之一
 */
ServerDialog::~ServerDialog()
{
    server.close();
    delete ui;
}


/**
 * 连接按钮点击事件
 * 点击之后开始监听端口
 */
void ServerDialog::on_pushButton_start_clicked()
{
    int pos = 0;//验证端口号是否为空用
    QString port = ui->lineedit_port->text();//输入端口号
    //检查端口号是否可用即是否在 0 - 65535 之间（QTcpServer自带）
    if(this->validator_port->validate(port,pos) == QIntValidator::Invalid
            || port.isEmpty()){
        QMessageBox::warning(this,tr("输入有误！"),tr("请输入端口号～\n要在 1 - 65535之间"));
        return;
    }
    else{
        //如果成功则跳转page
        ui->stackedWidget->setCurrentWidget(ui->page_2);
        //将端口号填入控件
        ui->lineEdit_Display_Port->setText(port);
        // 将端口号转换为无符号int类型并保存
        server.setPort(port.toUInt());
    }
    this->startServer();

}

/**
 *
 * @brief 客户端连接
 * @param ClientThread
 */
void ServerDialog::on_clientThread_connected(ClientThread* clientThread){

    ui->textEdit_ServerLog->append(tr("%1 连接成功～.").
                                   arg(clientThread->getSocketDescriptor()));

    //如果客户端请求成功的话，list列表中会添加一个item
    ui->listWidget_OnlineUser->addItem(
                QString::number(clientThread->getSocketDescriptor())+
                                       " : " + clientThread->getUsername());

}

/**
 * @brief 退出
 * @param clientThread
 */
void ServerDialog::on_clientThread_disconnected(ClientThread* clientThread){
    //获取套接字描述符打印到服务器屏幕
    ui->textEdit_ServerLog->append(tr("%1 退出～.").
                                   arg(clientThread->getSocketDescriptor()));
    //从列表删除用户
    QList<QListWidgetItem*> itemWidgets = ui->listWidget_OnlineUser->findItems(
                QString::number(clientThread->getSocketDescriptor())
                + " : " + clientThread->getUsername(),Qt::MatchExactly);
    foreach(QListWidgetItem* item,itemWidgets){
        delete item;
    }

}

void ServerDialog::on_clientThread_usernameChanged(int socketDescriptor,
                                                   QString uname){
    QString pattern = QString::number(socketDescriptor) + " : ";

    QList<QListWidgetItem*> itemWidgets = ui->listWidget_OnlineUser->findItems(
                QString::number(socketDescriptor) + " : ",Qt::MatchContains);
    foreach(QListWidgetItem* item,itemWidgets){
        item->setText(pattern + uname);
    }

}
/**
 * @brief 踢人按钮点击事件
 */
void ServerDialog::on_pushButton_Kick_clicked()
{
    //解决点击按钮窗口崩溃问题，如果点击时候没有用户，则return
    if(!ui->listWidget_OnlineUser->count()){
        return;
    }
    if(ui->listWidget_OnlineUser->selectedItems().count() == 0){
        return;
    }

    //从列表item中获取内容
    QString text = ui->listWidget_OnlineUser->currentItem()->text();

    //为输出的文件描述符设置正则表达式
    QRegularExpression rex("(\\d+) : ");
    QRegularExpressionMatch match = rex.match(text);

    int socketDescriptor = 0;
    //验证
    if(match.hasMatch()){
        socketDescriptor = match.captured(1).toInt();
        qDebug() << socketDescriptor;
        qDebug() << "++++++++++++++++++++++++++++";
    }

    server.kickFromServer(socketDescriptor);
//    emit pushButton_Kick_clicked(socketDescriptor);
}

/** 退出按钮点击事件*/
void ServerDialog::on_pushButton_Quit_clicked()
{
    this->close();
}

//! SLOTS



//! PRIVATE
/**
 * @brief server启动
 *
 * TODO：错误信息添加到日志文件
 */
void ServerDialog::startServer(){
    /* 启用服务器 */
    if(!server.StartServer()){
        QMessageBox::critical(this,tr("错误"),
                              tr("未能启动server: %1.").arg(server.errorString()));
        close();
        return;
    }

    /* 获取本地监听地址 */
    QString ipAddress;
    QList<QHostAddress> ipAddressList = QNetworkInterface::allAddresses();
    //use the first non-localhost IPv4 address.
    for(int i = 0; i < ipAddressList.size(); ++i){
        if(ipAddressList.at(i) != QHostAddress::LocalHost &&
           ipAddressList.at(i).toIPv4Address()){
            ipAddress = ipAddressList.at(i).toString();
            break;
        }
    }
    //如果没有则使用本地默认的ipv4端口（localhost）
    if(ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    //控件回传ip
    ui->lineEdit_IPAddr->setText(ipAddress);
    ui->textEdit_ServerLog->append("server等待连接中...\n");


}

