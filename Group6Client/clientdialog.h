#ifndef CLIENTDIALOG_H
#define CLIENTDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include "privatedialog.h"
#include <QString>
#include <QStringList>
#include <QIntValidator>
#include <QRegularExpression>
#include <QMap>
#include <QTime>
#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QTime>
#include <QLCDNumber>

/**
 * @class ClientDialog
 * @brief 客户端界面主类--继承 QDialog
 * @date 2018年12月17日
 */
namespace Ui {
class ClientDialog;
}

class ClientDialog : public QDialog
{
    Q_OBJECT

    //声明
public:
    explicit ClientDialog(QWidget *parent = 0);
    void sendFileToAll(QString filePath,QString filename);
    void sendTextToServer(QString text);
    void onFileReceived(QString senderName,QString filename,QByteArray dataOfFile);
    //用来关闭界面
    ~ClientDialog();


    //声明信号槽
public slots:
    void connected();
    //必须覆盖的方法之一，用来异步读取数据，信号槽（连接redyRead函数）
    void readyRead();
    void disconnected();
    void displayError(QAbstractSocket::SocketError socketError);
    void privateFinished(int result);
    void privateMessageSent(QString username,QString text);
    void privateFileSent(QString username,QString filePath,QString filename);


private slots:
    //连接按钮点击事件
    void on_pushButton_Login_clicked();
    //断开按钮点击事件
    void on_pushButton_Disconnect_clicked();

    void on_lineEdit_ChatInput_returnPressed();

    void on_pushButton_PrivateChat_clicked();
    void on_pushButton_ChooseFile_clicked();
    //显示时间
    void showTime();

private:
    Ui::ClientDialog *ui;
    //16bit 无符号整形
    quint16 port;
    QString username;
    //实体化QTcpSocket类
    QTcpSocket *tcpSocket;
    QString serverIP;
    //
    QMap<QString,PrivateDialog*> privateChatList;
    quint32 blockSize = 0;
    //定时器
    QTimer *myTimer;
    QLCDNumber *myLCDNumber;
};


#endif // CLIENTDIALOG_H
