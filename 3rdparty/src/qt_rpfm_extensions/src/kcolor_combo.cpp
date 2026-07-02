#include "kcolor_combo.h"

extern "C" int get_color(QWidget* view) {
#ifdef RPFM_NO_KDE
    return view->property("rpfmColor").toUInt();
#else
    KColorCombo* combo = dynamic_cast<KColorCombo*>(view);
    return combo->color().rgba();
#endif
}

extern "C" void set_color(QWidget* view, QColor* color) {
#ifdef RPFM_NO_KDE
    view->setProperty("rpfmColor", color->rgba());
    view->setStyleSheet(QString("background-color: %1").arg(color->name()));
#else
    KColorCombo* combo = dynamic_cast<KColorCombo*>(view);
    combo->setColor(*color);
    combo->update();
#endif
}
