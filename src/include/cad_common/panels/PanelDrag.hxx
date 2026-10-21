/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_PANEL_DRAG_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_PANEL_DRAG_HXX

#include <cstddef>
#include <optional>

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>
#include <QMimeData>
#include <QString>
#include <QtGlobal>

#include "PanelId.hxx"

/// @brief MIME payload shared by the panel drag source and drop targets
namespace panelDrag {

/// @brief MIME type carried while dragging a panel between sections
[[nodiscard]] inline const QString& mimeType() {
    static const auto type = QStringLiteral("application/x-cadcommon-panel");
    return type;
}

/// @brief Serialise @p id into the drag MIME payload
[[nodiscard]] inline QByteArray encode(const PanelId& id) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << static_cast<quint64>(id.hash) << QString::fromStdString(id.name);
    return data;
}

/// @brief Deserialize a panel id from @p mime when it carries one
[[nodiscard]] inline std::optional<PanelId> decode(const QMimeData* mime) {
    if (mime == nullptr || !mime->hasFormat(mimeType())) {
        return std::nullopt;
    }
    const QByteArray data = mime->data(mimeType());
    QDataStream stream(data);
    quint64 hash = 0;
    QString name;
    stream >> hash >> name;
    if (stream.status() != QDataStream::Ok) {
        return std::nullopt;
    }
    return PanelId{name.toStdString(), static_cast<std::size_t>(hash)};
}

} // namespace panelDrag

#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_PANEL_DRAG_HXX