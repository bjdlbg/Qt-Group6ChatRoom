#include "privatedialog.h"
#include "ui_privatedialog.h"

PrivateDialog::PrivateDialog(QString myUsername,QString username,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrivateDialog)
{
    ui->setupUi(this);
    this->username = username;
    //设置标题
    this->setWindowTitle("我与 "+username+" 的私聊窗口");
    this->myUsername = myUsername;
    //设置label内容
    ui->label_username->setText(this->username);

}

/** 结束窗口*/
PrivateDialog::~PrivateDialog()
{
    delete ui;
}

void PrivateDialog::addNewMessage(QString time, QString message){
    ui->textEdit_chatDisplay->append(
                "<b>"+username+" ["+time+"] </b>: "+message);
}

void PrivateDialog::on_lineEdit_chatInput_returnPressed()
{
    QString text = ui->lineEdit_chatInput->text().trimmed();

    if(text.isEmpty())
        return;
    //获取当前时间
    QString currentTime = QTime::currentTime().toString("H:m A");
    //添加字体font
    ui->textEdit_chatDisplay->append("<font color=\"MediumBlue\"><b>"+myUsername+" ["+currentTime+"] :</b> "+text +"</font>");

    //检查是否给自己发送
    if(this->myUsername != this->username){
        emit messageSent(username,text);
    }
    //每次回车之后清空输入框
    ui->lineEdit_chatInput->clear();
}

void PrivateDialog::on_pushButton_ChooseFile_clicked()
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
        //get current time string
        QString currentTime = QTime::currentTime().toString("H:mm A");
        //add to chat display
        ui->textEdit_chatDisplay->append("<font color=\"MediumBlue\"><b>"+myUsername+" ["+currentTime+"] send a file \""+filename+"\"</b></font>");
        emit fileSend(username,filePath,filename);
    }
}
