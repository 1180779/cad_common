/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_PANEL_ID_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_PANEL_ID_HXX

#include <cstddef>
#include <functional>
#include <string>
#include <utility>

#include <QMetaType>

/// @brief Stable identity of a panel hosted inside a PanelSection
/// @details The name is retained for diagnostics and user-facing text only.
/// Equality and hashing use a precomputed hash
struct PanelId {
    std::string name;
    std::size_t hash{};

    /// @brief Placeholder identity
    PanelId() = default;

    explicit PanelId(std::string id, const std::size_t hash) : name(std::move(id)), hash(hash) {
    }

    explicit PanelId(std::string id) : name(std::move(id)), hash(std::hash<std::string>{}(name)) {
    }

    explicit PanelId(const char* id) : PanelId(std::string(id)) {
    }

    friend bool operator==(const PanelId& lhs, const PanelId& rhs) noexcept { return lhs.hash == rhs.hash; }

    friend bool operator!=(const PanelId& lhs, const PanelId& rhs) noexcept { return lhs.hash != rhs.hash; }
};

template <>
struct std::hash<PanelId> {
    std::size_t operator()(const PanelId& id) const noexcept { return id.hash; }
};

Q_DECLARE_METATYPE(PanelId)

#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_PANEL_ID_HXX