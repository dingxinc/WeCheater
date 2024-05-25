#include "global.h"

std::function<void(QWidget*)> repolish = [](QWidget* w) {
    w->style()->unpolish(w);  // 先将样式卸载掉
    w->style()->polish(w);    // 再将样式装上，达到刷新的效果
};
