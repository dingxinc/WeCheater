#ifndef TCPMGR_H
#define TCPMGR_H
#include <QTcpSocket>
#include "singleton.h"
#include "global.h"

class TcpMgr : public QObject, public Singleton<TcpMgr>
        , public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT
public:
    ~TcpMgr();

private:
    friend class Singleton<TcpMgr>;
    TcpMgr();
void initHandlers();
    void handleMsg(ReqId id, int len, QByteArray data);
    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;         // 数据域
    bool _b_recv_pending;       // 数据是否收全的标志位，处理 tcp 粘包
    quint16 _message_id;
    quint16 _message_len;
    QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>> _handlers;// 收到请求的回调函数

public slots:
    void slot_tcp_connect(ServerInfo);   // 连接成功触发
    void slot_send_data(ReqId reqId, QString data);  // 数据发送成功触发

signals:
    void sig_con_success(bool bsuccess);  // 连接成功发送信号，告诉其他界面
    void sig_send_data(ReqId reqId, QString data);
    void sig_switch_chatdlg();
    void sig_login_failed(int);  // 登录失败发送信号
};

#endif // TCPMGR_H
