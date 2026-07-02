#ifndef QMAINWINDOWCUSTOM_H
#define QMAINWINDOWCUSTOM_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QEvent>
#include <QMessageBox>
#include <QStringList>
#ifdef RPFM_NO_KDE
#include <QProgressBar>
#else
#include <KBusyIndicatorWidget>
#endif

extern "C" QMainWindow* new_q_main_window_custom(bool (*are_you_sure)(QMainWindow* main_window, bool is_delete_my_mod, bool is_full_close) = nullptr, bool is_dark_theme_enabled = false);
extern "C" void flush_pending_open_packs(QMainWindow* main_window);

class QMainWindowCustom : public QMainWindow
{
    Q_OBJECT
public:
    explicit QMainWindowCustom(QWidget *parent = nullptr, bool (*are_you_sure)(QMainWindow* main_window, bool is_delete_my_mod, bool is_full_close) = nullptr, bool is_dark_theme_enabled = false);
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void flushPendingOpenPacks();

private:
    bool (*are_you_sure)(QMainWindow* main_window, bool is_delete_my_mod, bool is_full_close);
    bool dark_theme_enabled;
    QStringList pendingOpenPacks;
    void emitOrQueueOpenPack(QStringList const &paths);
#ifdef RPFM_NO_KDE
    QProgressBar* busyIndicator;
#else
    KBusyIndicatorWidget* busyIndicator;
#endif

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void openPack(QStringList const &);
    void themeChanged();

};

#endif // QMAINWINDOWCUSTOM_H
