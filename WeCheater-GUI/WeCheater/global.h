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

extern std::function<void(QWidget*)> repolish;  // 刷新 qss 样式

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

#endif // GLOBAL_H
