#ifndef HTTPMGR_H
#define HTTPMGR_H
#include "singleton.h"
#include <QString>
#include <QUrl>
#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>
#include <QJsonDocument>


class HttpMgr : public QObject, public Singleton<HttpMgr>,
        public std::enable_shared_from_this<HttpMgr>
{
    Q_OBJECT
public:
    ~HttpMgr();

private:
    friend class Singleton<HttpMgr>;  // 基类可以访问子类 HttpMgr 的构造函数
    HttpMgr();

public:
    /**
     * @brief PostHttpReq  发送 http 请求
     *    url 表示向哪个网络地址发送 http 请求
     *    json 表示发送的数据以 json 的格式传输
     *    req_id 表示是哪一个功能的请求
     *    mod 表示这个功能属于哪一个模块
     */
    void PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod);

private:
    QNetworkAccessManager _manager;

private slots:
    void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);

signals:
    void sig_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
    void sig_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
};

#endif // HTTPMGR_H
