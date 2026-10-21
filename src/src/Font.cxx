/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#include "../include/cad_common/Font.hxx"

#include <QApplication>
#include <QFont>
#include <QFontDatabase>

// must live at global scope: Q_INIT_RESOURCE declares qInitResources_<name>
static void initFontResources() {
    Q_INIT_RESOURCE(styles);
}

namespace {

    QString loadJetBrainsMono() {
        initFontResources();

        QFontDatabase::addApplicationFont(QStringLiteral(":/cad_common/fonts/JetBrainsMono-Bold.ttf"));
        const int regularId = QFontDatabase::addApplicationFont(
            QStringLiteral(":/cad_common/fonts/JetBrainsMono-Regular.ttf"));
        if (regularId == -1) {
            return {};
        }
        const QStringList families = QFontDatabase::applicationFontFamilies(regularId);
        return families.isEmpty() ? QString() : families.first();
    }

} // namespace

namespace cad_common {

QString applyAppFont(const int pointSize) {
    static const QString family = loadJetBrainsMono();
    if (family.isEmpty()) {
        return {};
    }

    QFont font(family);
    font.setPointSize(pointSize);
    QApplication::setFont(font);
    return family;
}

} // namespace cad_common
