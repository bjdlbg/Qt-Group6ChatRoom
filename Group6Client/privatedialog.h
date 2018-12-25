#ifndef PRIVATEDDIALOG_H
#define PRIVATEDDIALOG_H


#include <QDialog>
#include <QString>
#include <QTime>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

/**
 * @class PrivateDialog
 * @brief 私聊界面窗口
 * 该类声明所需要的控件，对象，函数等
 */
namespace Ui {
class PrivateDialog;
}

class PrivateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrivateDialog(QString myUsername,QString username,QWidget *parent = 0);
    QString getUsername() const{return username;}
    void addNewMessage(QString time,QString message);
    ~PrivateDialog();

signals:
    void messageSent(QString username,QString message); //when user press enter in chat input
    void fileSend(QString username,QString filePath,QString filename);


private slots:
    void on_lineEdit_chatInput_returnPressed();

    void on_pushButton_ChooseFile_clicked();

private:
    Ui::PrivateDialog *ui;
    QString username;
    QString myUsername;
};

#endif // PRIVATEDDIALOG_H
