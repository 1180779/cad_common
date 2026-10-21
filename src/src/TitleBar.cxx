/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#include "../include/cad_common/TitleBar.hxx"

#include <QColor>
#include <QEvent>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMouseEvent>
#include <QStyle>
#include <QToolButton>
#include <QWindow>

#include "../include/cad_common/Icon.hxx"

namespace {

constexpr int gc_titleBarHeight = 30;
constexpr int gc_windowButtonIconSize = 14;

QColor windowTextColor(const QWidget* widget) {
    return widget->palette().color(QPalette::WindowText);
}

QToolButton* makeWindowButton(QWidget* parent, const QString& iconName) {
    auto* button = new QToolButton(parent);
    button->setIcon(cad_common::recoloredIcon(iconName, windowTextColor(parent)));
    button->setIconSize(QSize(gc_windowButtonIconSize, gc_windowButtonIconSize));
    button->setAutoRaise(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setFixedSize(gc_titleBarHeight, gc_titleBarHeight);
    button->setCursor(Qt::ArrowCursor);
    button->setObjectName(QStringLiteral("windowButton"));
    return button;
}

/// @brief Drives native system-resize from the border of a frameless window
class FramelessResizeFilter final : public QObject {
public:
    FramelessResizeFilter(QWidget* window, const int margin)
        : QObject(window), m_window(window), m_margin(margin) {
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
        if (watched != m_window) {
            return QObject::eventFilter(watched, event);
        }

        switch (event->type()) {
        case QEvent::MouseMove: {
            // don't offer a resize cursor on platforms that refused the gesture
            if (m_resizeSupported && !m_window->isMaximized()) {
                const auto* me = dynamic_cast<QMouseEvent*>(event);
                if (const Qt::Edges edges = edgesAt(me->position().toPoint()); edges != Qt::Edges()) {
                    m_window->setCursor(cursorForEdges(edges));
                } else {
                    m_window->unsetCursor();
                }
            }
            break;
        }
        case QEvent::Leave:
            m_window->unsetCursor();
            break;
        case QEvent::MouseButtonPress: {
            if (const auto* me = dynamic_cast<QMouseEvent*>(event);
                me->button() == Qt::LeftButton && m_resizeSupported && !m_window->isMaximized()) {
                if (const Qt::Edges edges = edgesAt(me->position().toPoint()); edges != Qt::Edges()) {
                    if (QWindow* handle = m_window->windowHandle()) {
                        // consume the press only if the platform actually began the resize
                        if (handle->startSystemResize(edges)) {
                            return true;
                        }
                        m_resizeSupported = false;
                        m_window->unsetCursor();
                    }
                }
            }
            break;
        }
        default:
            break;
        }
        return QObject::eventFilter(watched, event);
    }

private:
    QWidget* m_window;
    int m_margin;

    /// @brief Latched off if the platform refuses startSystemResize
    bool m_resizeSupported = true;

    [[nodiscard]] Qt::Edges edgesAt(const QPoint& pos) const {
        Qt::Edges edges;
        if (pos.x() <= m_margin) {
            edges |= Qt::LeftEdge;
        }
        if (pos.x() >= m_window->width() - m_margin) {
            edges |= Qt::RightEdge;
        }
        if (pos.y() <= m_margin) {
            edges |= Qt::TopEdge;
        }
        if (pos.y() >= m_window->height() - m_margin) {
            edges |= Qt::BottomEdge;
        }
        return edges;
    }

    [[nodiscard]] static Qt::CursorShape cursorForEdges(const Qt::Edges edges) {
        if ((edges & Qt::LeftEdge && edges & Qt::TopEdge) || (edges & Qt::RightEdge && edges & Qt::BottomEdge)) {
            return Qt::SizeFDiagCursor;
        }
        if ((edges & Qt::RightEdge && edges & Qt::TopEdge) || (edges & Qt::LeftEdge && edges & Qt::BottomEdge)) {
            return Qt::SizeBDiagCursor;
        }
        if (edges & (Qt::LeftEdge | Qt::RightEdge)) {
            return Qt::SizeHorCursor;
        }
        if (edges & (Qt::TopEdge | Qt::BottomEdge)) {
            return Qt::SizeVerCursor;
        }
        return Qt::ArrowCursor;
    }
};

} // namespace

TitleBar::TitleBar(QWidget* parent)
    : QWidget(parent) {
    setObjectName(QStringLiteral("windowTitleBar"));
    setFixedHeight(gc_titleBarHeight);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 0, 0, 0);
    layout->setSpacing(0);

    m_menuBar = new QMenuBar(this);
    m_menuBar->setCursor(Qt::ArrowCursor);
    layout->addWidget(m_menuBar);

    // Empty drag region between the menu and the window buttons
    auto* filler = new QWidget(this);
    filler->setCursor(Qt::ArrowCursor);
    filler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget(filler, 1);

    m_minButton = makeWindowButton(this, QStringLiteral("window-minimize"));
    m_maxButton = makeWindowButton(this, QStringLiteral("window-maximize"));
    m_closeButton = makeWindowButton(this, QStringLiteral("window-close"));
    m_closeButton->setProperty("close", true);
    m_closeButton->installEventFilter(this);
    layout->addWidget(m_minButton);
    layout->addWidget(m_maxButton);
    layout->addWidget(m_closeButton);

    connect(m_minButton, &QToolButton::clicked, this, [this] { window()->showMinimized(); });
    connect(m_maxButton, &QToolButton::clicked, this, [this] { toggleMaximized(); });
    connect(m_closeButton, &QToolButton::clicked, this, [this] { window()->close(); });
}

QMenuBar* TitleBar::menuBar() const {
    return m_menuBar;
}

void TitleBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        // take over the press only if the platform actually began the move
        if (QWindow* handle = window()->windowHandle(); handle && handle->startSystemMove()) {
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        toggleMaximized();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

bool TitleBar::event(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange) {
        updateMaximizeButton();
    } else if (event->type() == QEvent::PaletteChange) {
        refreshButtonIcons();
    }
    return QWidget::event(event);
}

bool TitleBar::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_closeButton) {
        // white glyph on the red hover/press background
        if (event->type() == QEvent::Enter) {
            m_closeButton->setIcon(cad_common::recoloredIcon(QStringLiteral("window-close"), QColor(Qt::white)));
        } else if (event->type() == QEvent::Leave) {
            m_closeButton->setIcon(
                cad_common::recoloredIcon(QStringLiteral("window-close"), windowTextColor(m_closeButton)));
        }
    }
    return QWidget::eventFilter(watched, event);
}

void TitleBar::toggleMaximized() const {
    QWidget* w = window();
    if (w->isMaximized()) {
        w->showNormal();
    } else {
        w->showMaximized();
    }
    updateMaximizeButton();
}

void TitleBar::updateMaximizeButton() const {
    m_maxButton->setIcon(cad_common::recoloredIcon(
        window()->isMaximized() ? QStringLiteral("window-restore") : QStringLiteral("window-maximize"),
        windowTextColor(m_maxButton)));
}

void TitleBar::refreshButtonIcons() const {
    m_minButton->setIcon(
        cad_common::recoloredIcon(QStringLiteral("window-minimize"), windowTextColor(m_minButton)));
    m_closeButton->setIcon(cad_common::recoloredIcon(
        QStringLiteral("window-close"),
        m_closeButton->underMouse() ? QColor(Qt::white) : windowTextColor(m_closeButton)));
    updateMaximizeButton();
}

void enableFramelessResize(QWidget* window, const int margin) {
    window->setMouseTracking(true);
    window->installEventFilter(new FramelessResizeFilter(window, margin));
}