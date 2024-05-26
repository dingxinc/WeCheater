#include "httpmgr.h"

HttpMgr::~HttpMgr()
{

}

HttpMgr::HttpMgr()
{
    connect(this, &HttpMgr::sig_http_finish, this, &HttpMgr::slot_http_finish);
}

void HttpMgr::PostHttpReq(QUrl url, QJsonObject json, ReqId req_id, Modules mod)
{
    QByteArray data = QJsonDocument(json).toJson();  // 将 json 对象转为 字节数组
    QNetworkRequest request(url);     // 设置 url
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");  // 设置 http 请求头
    request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(data.length())); // 数据长度
    auto self = shared_from_this();    // 伪闭包
    /* 发送请求后，会收到回应 */
    QNetworkReply * reply = _manager.post(request, data);  // 发送 post 请求
    /* 当我们收到回应以后，会触发 QNetworkReply::finish 的信号 */
    QObject::connect(reply, &QNetworkReply::finished, [self, reply, req_id, mod]() {
        // 有错误
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << reply->errorString();
            // 发送信号通知完成
            emit self->sig_http_finish(req_id, "", ErrorCodes::ERR_NETWORK, mod);  // 发送信号通知其他模块发生网络错误
            reply->deleteLater();  // reply 是 new 出来的，需要回收
            return;
        }
        // 无错误
        QString req = reply->readAll();  // 读出所有的回应字节
        // 发送信号通知完成
        emit self->sig_http_finish(req_id, req, ErrorCodes::SUCCESS, mod);
        reply->deleteLater();
        return;
    });
}

void HttpMgr::slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod)
{
    if (mod == Modules::REGISTERMOD) {
        // 发送信号通知执行模块 http 的响应结束了
        emit sig_reg_mod_finish(id, res, err);
    }

    if(mod == Modules::RESETMOD){
        //发送信号通知指定模块http响应结束
        emit sig_reset_mod_finish(id, res, err);
    }

//    if(mod == Modules::LOGINMOD){
//        emit sig_login_mod_finish(id, res, err);
//    }
}
