/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_TITLE_BAR_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_TITLE_BAR_HXX

#include <QWidget>

class QMenuBar;
class QToolButton;

namespace cadc {

/// @brief IntelliJ-style unified title bar for a frameless window
///
/// Hosts a menu bar on the left and minimize / maximize / close controls on the
/// right. The empty region drags the window; double-clicking it toggles the
/// maximized state. Pair with enableFramelessResize()
class TitleBar final : public QWidget {
    Q_OBJECT

public:
    explicit TitleBar(QWidget* parent = nullptr);

    /// @brief The menu bar hosted by the title bar
    [[nodiscard]] QMenuBar* menuBar() const;

    bool eventFilter(QObject* watched, QEvent* event) override;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    bool event(QEvent* event) override;

private:
    void toggleMaximized() const;
    void updateMaximizeButton() const;
    void refreshButtonIcons() const;

    QMenuBar* m_menuBar;
    QToolButton* m_minButton;
    QToolButton* m_maxButton;
    QToolButton* m_closeButton;
};

/// @brief Install resize handling on a frameless top-level @p window
///
/// Watches the @p margin-wide border for hover/press and drives the platform's
/// native system-resize, so a FramelessWindowHint window stays resizable
void enableFramelessResize(QWidget* window, int margin = 6);

} // namespace cadc

#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_TITLE_BAR_HXX