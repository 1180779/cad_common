/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_ICON_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_ICON_HXX

#include <QIcon>

namespace cad_common {

/// @brief Load a bundled SVG icon, recolored to @p color
/// @details Icons are embedded under
/// <tt>:/cad_common/icons/&lt;name&gt;.svg</tt> and use @c currentColor, which
/// is replaced by @p color. The recolored SVG is cached on the drive
[[nodiscard]] QIcon recoloredIcon(const QString& name, const QColor& color);
}

#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_ICON_HXX