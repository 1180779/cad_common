/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_MULTI_SIGNAL_BLOCKER_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_MULTI_SIGNAL_BLOCKER_HXX

#include <QObject>
// ReSharper disable once CppUnusedIncludeDirective
#include <QSignalBlocker>

#include <array>
#include <concepts>
#include <cstddef>

template <std::size_t N>
class MultiSignalBlocker final {
public:
    template <typename... Objects>
        requires (sizeof...(Objects) == N && (std::convertible_to<Objects*, QObject*> && ...))
    explicit MultiSignalBlocker(Objects*... objects)
        : m_blockers{QSignalBlocker{objects}...} {
    }

    MultiSignalBlocker(const MultiSignalBlocker&) = delete;
    MultiSignalBlocker& operator=(const MultiSignalBlocker&) = delete;

private:
    std::array<QSignalBlocker, N> m_blockers;
};

template <typename... Objects>
MultiSignalBlocker(Objects*...) -> MultiSignalBlocker<sizeof...(Objects)>;

#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_MULTI_SIGNAL_BLOCKER_HXX