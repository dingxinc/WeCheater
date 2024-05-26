#include "logindialog.h"
#include "ui_logindialog.h"
#include <QDebug>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    // 注册按钮发出 clicked 信号，我们接收后发送 switchRegister 信号，信号是可以和信号链接的
    connect(ui->reg_btn, &QPushButton::clicked, this, &LoginDialog::switchRegister);

    ui->forget_label->SetState("normal","hover","","selected","selected_hover","");
    connect(ui->forget_label, &ClickedLabel::clicked, this, &LoginDialog::slot_forget_pwd);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::slot_forget_pwd()
{
    qDebug()<<"slot forget pwd";
    emit switchReset();
}
