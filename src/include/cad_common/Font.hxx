/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_FONT_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_FONT_HXX

#include <QString>

namespace cad_common {

/// @brief Load the bundled JetBrains Mono family and make it the application
/// default font
/// @param pointSize the application font size
/// @returns the resolved family name, or an empty string on failure
QString applyAppFont(int pointSize = 11);
}

#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_FONT_HXX
