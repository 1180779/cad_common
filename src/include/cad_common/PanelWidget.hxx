/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_PANEL_WIDGET_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_PANEL_WIDGET_HXX

#include <string>
#include <utility>

#include <QLayout>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

/// @brief Base class for content hosted inside a PanelSection
/// @details Carries a display title shown in the section header. The title is
/// read when the panel is opened; changing it later does not refresh an
/// already-open header
class PanelWidget : public QWidget {
    Q_OBJECT

public:
    explicit PanelWidget(std::string title, QWidget* parent = nullptr);

    [[nodiscard]] const std::string& title() const;
    void setTitle(std::string title);

protected:
    /// @brief A QScrollArea whose content resizes with the viewport and hides
    /// the horizontal scrollbar
    QScrollArea* createScroll();

    QScrollArea* createScroll(QWidget* widget);

    /// @brief A layout with the standard panel margins and top alignment
    /// @note TLayout must derive from QLayout
    template <typename TLayout = QVBoxLayout>
    TLayout* createLayout(Qt::AlignmentFlag alignment = Qt::AlignTop) {
        auto* layout = new TLayout(this);
        layout->setContentsMargins(6, 6, 6, 6);
        layout->setAlignment(alignment);
        return layout;
    }

    /// @brief A layout whose alignment is unset (suitable for scroll contents)
    template <typename TLayout = QVBoxLayout>
    TLayout* createScrollLayout() {
        return createLayout<TLayout>(Qt::AlignmentFlag{});
    }

    /// @brief A scrollable layout wrapping @p widget
    template <typename TLayout = QVBoxLayout>
    TLayout* createScrollLayout(QWidget* widget) {
        auto* layout = createScrollLayout<TLayout>();
        layout->addWidget(createScroll(widget));
        return layout;
    }

private:
    std::string m_title;
};

#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_PANEL_WIDGET_HXX