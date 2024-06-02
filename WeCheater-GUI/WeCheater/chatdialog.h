#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    /* 测试函数 */
    void addChatUserList();

private:
    void ShowSearch(bool bsearch = false);
    Ui::ChatDialog *ui;

    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;   // 负责加载的成员变量
};

#endif // CHATDIALOG_H
