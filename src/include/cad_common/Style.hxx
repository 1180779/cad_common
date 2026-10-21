/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_STYLE_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_STYLE_HXX

namespace cad_common {
/// @brief Which preconfigured look to apply
enum class StyleMode {
    Light, ///< Light palette + stylesheet
    Dark, ///< Dark palette + stylesheet
    System, ///< Follow the running platform's color scheme
};

/// @brief Helpers for applying the bundled application look
namespace style {
/// @brief Apply a preconfigured light/dark palette and the matching
/// bundled stylesheet to the running QApplication
/// @details Call once after the QApplication is constructed; safe to
/// call again to switch modes. No-op if there is no QApplication yet
void apply(StyleMode mode = StyleMode::System);
}
}

#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_STYLE_HXX