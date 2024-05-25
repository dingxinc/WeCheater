#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include "httpmgr.h"

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->pass_edit->setEchoMode(QLineEdit::Password);
    ui->confirm_edit->setEchoMode(QLineEdit::Password);

    // 设置 err_tip 的属性
    ui->err_tip->setProperty("state", "normal");  // 初始是正常状态
    repolish(ui->err_tip);   // 刷新 err_tip 的样式

    // http 发送信号，通知注册模块请求完成了，用裸指针发送信号
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish,
            this, &RegisterDialog::slot_reg_mod_finish);

    // 调用 http 处理函数
    initHttpHandlers();
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_get_code_clicked()
{
    auto email = ui->email_edit->text();   // 获取邮箱的文本内容
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(match){
        //发送http请求获取验证码
        QJsonObject json_obj;
        json_obj["email"] = email;  // 将匹配成功的 email 通过 json 发到服务端
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix + "/get_varifycode"), json_obj
                                            , ReqId::ID_GET_VARIFY_CODE, Modules::REGISTERMOD);
    }else{
        //提示邮箱不正确
        showTip(tr("邮箱地址不正确"), false);
    }
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if (err != ErrorCodes::SUCCESS) {
        showTip(tr("网络请求错误"), false);
        return;
    }

    // 解析 JSON 字符串， res 转换为 QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());  // xxx.json json 文档
    if (jsonDoc.isNull()) {
        showTip(tr("json解析错误"), false);
        return;
    }

    if (!jsonDoc.isObject()) {
        showTip(tr("json解析错误"), false);
        return;
    }

    // 回调函数执行
    _handlers[id](jsonDoc.object());  // json 文档转为 json 对象传到回调函数中
    return;
}

void RegisterDialog::showTip(QString str, bool b_ok)
{
    if (b_ok) {
        ui->err_tip->setProperty("state", "normal");
    } else {
        ui->err_tip->setProperty("state", "err");
    }
    ui->err_tip->setText(str);
    repolish(ui->err_tip);
}

void RegisterDialog::initHttpHandlers()
{
// 注册获取验证码回包的逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](const QJsonObject& jsonObj){
        int error = jsonObj["error"].toInt();
        if (error != ErrorCodes::SUCCESS) {
            showTip(tr("参数错误"), false);
            return;
        }

        // 服务器会返回一个 email
        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已发送至邮箱，请注意查收"), true);
        qDebug() << "email is " << email;
    });
}
