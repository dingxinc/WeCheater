#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "logindialog.h"
#include "registerdialog.h"
#include "resetdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void SlotSwitchReg();
    void SlotSwitchLogin();     // 从注册界面回到登录界面
    void SlotSwitchReset();
    void SlotSwitchLogin2();    // 从重置界面回到登录界面

private:
    Ui::MainWindow *ui;
    LoginDialog * _login_dlg;   // 登录界面
    RegisterDialog * _reg_dlg;  // 注册界面
    ResetDialog * _reset_dlg;   // 重置密码界面
};
#endif // MAINWINDOW_H
