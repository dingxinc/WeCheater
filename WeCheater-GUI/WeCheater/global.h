#ifndef GLOBAL_H
#define GLOBAL_H
#include <QWidget>
#include <functional>
#include "QStyle"
#include <regex>
#include <QRegularExpression>
#include <memory>
#include <mutex>
#include <iostream>
#include <QByteArray>
#include <QNetworkReply>
#include <QJsonObject>
#include <QDir>
#include <QFile>
#include <QSettings>   // 读取 .ini 配置文件使用

extern std::function<void(QWidget*)> repolish;  // 刷新 qss 样式

extern std::function<QString(QString)> xorString; // 异或密码

/* 功能 id */
enum ReqId {
    ID_GET_VARIFY_CODE = 1001,   // 获取验证码
    ID_REG_USER = 1002           // 注册用户
};

/* 模块 ，功能属于哪一个 模块 */
enum Modules {
    REGISTERMOD = 0   // 注册模块
};

/* 错误码 */
enum ErrorCodes {
    SUCCESS = 0,
    ERR_JSON = 1,   // json 解析错误
    ERR_NETWORK = 2 // 网络错误
};

// GateServer 的 url 前缀
extern QString gate_url_prefix;

#endif // GLOBAL_H
