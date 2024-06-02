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
#include <QJsonDocument>

extern std::function<void(QWidget*)> repolish;  // 刷新 qss 样式

extern std::function<QString(QString)> xorString; // 异或密码

/* 功能 id */
enum ReqId {
    ID_GET_VARIFY_CODE = 1001,   // 获取验证码
    ID_REG_USER = 1002,           // 注册用户
    ID_RESET_PWD = 1003, //重置密码
    ID_LOGIN_USER = 1004, //用户登录
    ID_CHAT_LOGIN = 1005, //登陆聊天服务器
    ID_CHAT_LOGIN_RSP= 1006, //登陆聊天服务器回包
};

/* 模块 ，功能属于哪一个 模块 */
enum Modules {
    REGISTERMOD = 0,   // 注册模块
    RESETMOD = 1,       // 重置模块
    LOGINMOD = 2        // 登录模块
};

/* 错误码 */
enum ErrorCodes {
    SUCCESS = 0,
    ERR_JSON = 1,   // json 解析错误
    ERR_NETWORK = 2 // 网络错误
};

enum TipErr {
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_CONFIRM_ERR = 3,
    TIP_PWD_CONFIRM = 4,
    TIP_VARIFY_ERR = 5,
    TIP_USER_ERR = 6
};

enum ClickLbState {
    Normal = 0,
    Selected = 1
};

//聊天界面几种模式
enum ChatUIMode {
    SearchMode, //搜索模式
    ChatMode, //聊天模式
    ContactMode // 联系模式
};

//自定义QListWidgetItem的几种类型
enum ListItemType {
    CHAT_USER_ITEM, //聊天用户
    CONTACT_USER_ITEM, //联系人用户
    SEARCH_USER_ITEM, //搜索到的用户
    ADD_USER_TIP_ITEM, //提示添加用户
    INVALID_ITEM,  //不可点击条目
    GROUP_TIP_ITEM // 分组提示条目
};

struct ServerInfo {
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};

// GateServer 的 url 前缀
extern QString gate_url_prefix;

#endif // GLOBAL_H
