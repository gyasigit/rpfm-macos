#include "q_main_window_custom.h"
#include <QApplication>
#include <QDebug>
#include <QFileOpenEvent>
#include <QFileInfo>
#include <QIcon>
#include <QMimeData>
#include <QResource>
#include <QStatusBar>

#ifndef RPFM_NO_KDE
#include "kicontheme.h"
#include <breezeicons.h>
#endif

// Must be called before QApplication is created. Sets up KIconTheme so that the
// KIconEnginePlugin is discovered and icons are palette-recolored on dark themes.
extern "C" void init_icon_theme() {
#ifndef RPFM_NO_KDE
    KIconTheme::initTheme();
#endif
}

// Fuction to be able to create a custom QMainWindow.
extern "C" QMainWindow* new_q_main_window_custom(bool (*are_you_sure) (QMainWindow* main_window, bool is_delete_my_mod, bool is_full_close), bool is_dark_theme_enabled) {
    return dynamic_cast<QMainWindow*>(new QMainWindowCustom(nullptr, are_you_sure, is_dark_theme_enabled));
}

extern "C" void flush_pending_open_packs(QMainWindow* main_window) {
    if (auto customWindow = dynamic_cast<QMainWindowCustom*>(main_window)) {
        customWindow->flushPendingOpenPacks();
    }
}

QMainWindowCustom::QMainWindowCustom(QWidget *parent, bool (*are_you_sure_fn) (QMainWindow* main_window, bool is_delete_my_mod, bool is_full_close), bool is_dark_theme_enabled) : QMainWindow(parent) {
    are_you_sure = are_you_sure_fn;
    dark_theme_enabled = is_dark_theme_enabled;

#ifdef RPFM_NO_KDE
    busyIndicator = new QProgressBar();
    busyIndicator->setRange(0, 0);
    busyIndicator->setTextVisible(false);
    busyIndicator->setFixedSize(48, 14);
#else
    busyIndicator = new KBusyIndicatorWidget();
    busyIndicator->setFixedSize(16, 16);
#endif
    statusBar()->addPermanentWidget(busyIndicator);
    busyIndicator->hide();

    setAcceptDrops(true);
    qApp->installEventFilter(this);

    #ifdef _WIN32

        // Initialize the Breeze icon theme from the KF6BreezeIcons library.
        // This registers both breeze and breeze-dark themes from the compiled-in
        // resources, and Qt's icon engine handles dark/light switching automatically
        // based on the current palette.
        BreezeIcons::initIcons();
        QIcon::setThemeName(QStringLiteral("breeze"));
    #endif
}

void QMainWindowCustom::emitOrQueueOpenPack(QStringList const &paths) {
    if (paths.isEmpty()) {
        return;
    }

    if (receivers(SIGNAL(openPack(QStringList const &))) > 0) {
        emit openPack(paths);
    } else {
        pendingOpenPacks.append(paths);
    }
}

void QMainWindowCustom::flushPendingOpenPacks() {
    if (pendingOpenPacks.isEmpty()) {
        return;
    }

    QStringList paths;
    paths.swap(pendingOpenPacks);
    emit openPack(paths);
}

// Overload of the close event so we can put a dialog there.
void QMainWindowCustom::closeEvent(QCloseEvent *event) {
    event->ignore();

    if (are_you_sure && are_you_sure(this, false, true)) {
        event->accept();
    }
}

void QMainWindowCustom::changeEvent(QEvent* event) {
    if (event->type() == QEvent::EnabledChange) {
        if (isEnabled()) {
            busyIndicator->hide();
        } else {
            busyIndicator->show();
        }
    }

    // Notify Rust side so it can update theme-dependent widgets
    if (event->type() == QEvent::PaletteChange) {
        emit themeChanged();
    }

    QMainWindow::changeEvent(event);
}

bool QMainWindowCustom::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::FileOpen) {
        auto fileOpenEvent = static_cast<QFileOpenEvent*>(event);
        QString filePath = fileOpenEvent->file();

        if (filePath.isEmpty()) {
            filePath = fileOpenEvent->url().toLocalFile();
        }

        if (!filePath.isEmpty()) {
            emitOrQueueOpenPack(QStringList{filePath});
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void QMainWindowCustom::dragEnterEvent(QDragEnterEvent *event) {
    event->accept();
}

void QMainWindowCustom::dragMoveEvent(QDragMoveEvent *event) {
    QMainWindow::dragMoveEvent(event);
}

void QMainWindowCustom::dragLeaveEvent(QDragLeaveEvent *event) {
    QMainWindow::dragLeaveEvent(event);
}

void QMainWindowCustom::dropEvent(QDropEvent *event) {

    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();

        for (int i = 0; i < urlList.size(); ++i) {
            pathList.append(urlList.at(i).toLocalFile());
        }

        emitOrQueueOpenPack(pathList);
    }
}
