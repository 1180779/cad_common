/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#include "../include/cad_common/Icon.hxx"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QSaveFile>
#include <QStandardPaths>

// must live at global scope: Q_INIT_RESOURCE declares qInitResources_<name>
static void initIconResources() {
    Q_INIT_RESOURCE(styles);
}

namespace {

const QString& iconCacheDir() {
    static const QString dir = [] {
        const QString path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
                             + QStringLiteral("/cad_common/icons");
        return QDir().mkpath(path) ? path : QString();
    }();
    return dir;
}

} // namespace

namespace cad_common {

QIcon recoloredIcon(const QString& name, const QColor& color) {
    static const bool resourcesReady = [] {
        initIconResources();
        return true;
    }();
    Q_UNUSED(resourcesReady)

    const QString source = QStringLiteral(":/cad_common/icons/%1.svg").arg(name);
    const QString& cache = iconCacheDir();
    if (cache.isEmpty()) {
        return QIcon(source);
    }

    const QString output = QStringLiteral("%1/%2-%3.svg").arg(cache, name, color.name().mid(1));
    if (QFileInfo::exists(output)) {
        return QIcon(output);
    }

    QFile input(source);
    if (!input.open(QIODevice::ReadOnly)) {
        return QIcon(source);
    }
    QByteArray svg = input.readAll();
    svg.replace("currentColor", color.name().toUtf8());
    if (QSaveFile file(output); file.open(QIODevice::WriteOnly) && file.write(svg) == svg.size()
                                && file.commit()) {
        return QIcon(output);
    }
    return QIcon(source);
}

} // namespace cad_common