#include "kmessage_widget.h"
#include <QMessageBox>
#include <QIcon>

#ifdef RPFM_NO_KDE
static QLabel* label_from_widget(QWidget* widget) {
    return dynamic_cast<QLabel*>(widget);
}

static void set_message(QWidget* widget, QString const text, QString const level) {
    QLabel* label = label_from_widget(widget);
    if (label == nullptr) {
        return;
    }

    label->hide();
    label->setText(text);
    label->setProperty("rpfmMessageLevel", level);
    label->setStyleSheet("QLabel { padding: 6px; border: 1px solid palette(mid); border-radius: 3px; }");
    label->show();
}

extern "C" QWidget* kmessage_widget_new(QWidget* widget) {
    QLabel* label = new QLabel(widget);
    label->setWordWrap(true);
    label->hide();
    return label;
}

extern "C" void kmessage_widget_close(QWidget* widget) {
    QLabel* label = label_from_widget(widget);
    if (label != nullptr) {
        label->hide();
    }
}

extern "C" bool kmessage_widget_is_closed(QWidget* widget) {
    QLabel* label = label_from_widget(widget);
    return label == nullptr || label->isHidden();
}

extern "C" void kmessage_widget_set_error(QWidget* widget, QString const text) {
    set_message(widget, text, "error");
}

extern "C" void kmessage_widget_set_warning(QWidget* widget, QString const text) {
    set_message(widget, text, "warning");
}

extern "C" void kmessage_widget_set_info(QWidget* widget, QString const text) {
    set_message(widget, text, "info");
}
#else
extern "C" QWidget* kmessage_widget_new(QWidget* widget) {
    KMessageWidget* kmessagewidget = new KMessageWidget(widget);
    kmessagewidget->setWordWrap(true);
    kmessagewidget->hide();
    return kmessagewidget;
}

extern "C" void kmessage_widget_close(QWidget* widget) {
    KMessageWidget* kmessagewidget = dynamic_cast<KMessageWidget*>(widget);
    kmessagewidget->setWordWrap(true);
    kmessagewidget->hide();
}

extern "C" bool kmessage_widget_is_closed(QWidget* widget) {
    KMessageWidget* kmessagewidget = dynamic_cast<KMessageWidget*>(widget);
    return kmessagewidget->isHidden();
}

extern "C" void kmessage_widget_set_error(QWidget* widget, QString const text) {
    KMessageWidget* kmessagewidget = dynamic_cast<KMessageWidget*>(widget);
    kmessagewidget->hide();
    kmessagewidget->setText(text);
    kmessagewidget->setMessageType(KMessageWidget::MessageType::Error);
    kmessagewidget->setIcon(QIcon::fromTheme("dialog-error"));
    kmessagewidget->animatedShow();
}

extern "C" void kmessage_widget_set_warning(QWidget* widget, QString const text) {
    KMessageWidget* kmessagewidget = dynamic_cast<KMessageWidget*>(widget);
    kmessagewidget->hide();
    kmessagewidget->setText(text);
    kmessagewidget->setMessageType(KMessageWidget::MessageType::Warning);
    kmessagewidget->setIcon(QIcon::fromTheme("dialog-warning"));
    kmessagewidget->animatedShow();
}

extern "C" void kmessage_widget_set_info(QWidget* widget, QString const text) {
    KMessageWidget* kmessagewidget = dynamic_cast<KMessageWidget*>(widget);
    kmessagewidget->hide();
    kmessagewidget->setText(text);
    kmessagewidget->setMessageType(KMessageWidget::MessageType::Information);
    kmessagewidget->setIcon(QIcon::fromTheme("dialog-information"));
    kmessagewidget->animatedShow();
}
#endif
