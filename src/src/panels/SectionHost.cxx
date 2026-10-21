/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#include <cad_common/panels/SectionHost.hxx>

#include <algorithm>

#include <QColor>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QRect>
#include <QSplitter>
#include <QVBoxLayout>

#include <cad_common/panels/PanelDrag.hxx>
#include <cad_common/panels/PanelWidget.hxx>

namespace {
void configureSplitter(QSplitter* splitter) {
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(4);
}
} // namespace

/// @brief Translucent accent highlight shown over the section a drag would
/// drop into
class SectionHost::DropOverlay final : public QWidget {
public:
    explicit DropOverlay(QWidget* parent = nullptr)
        : QWidget(parent) {
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
        hide();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        const QColor accent = palette().color(QPalette::Highlight);
        QColor fill = accent;
        fill.setAlpha(60);
        painter.fillRect(rect(), fill);
        painter.setPen(QPen(accent, 2));
        painter.drawRect(rect().adjusted(1, 1, -2, -2));
    }
};

SectionHost::SectionHost(QWidget* central, QWidget* parent)
    : QWidget(parent), m_central(central) {
    m_sections[indexOf(SectionSide::Left)] = new PanelSection(SectionSide::Left);
    m_sections[indexOf(SectionSide::Right)] = new PanelSection(SectionSide::Right);
    m_sections[indexOf(SectionSide::Top)] = new PanelSection(SectionSide::Top);
    m_sections[indexOf(SectionSide::Bottom)] = new PanelSection(SectionSide::Bottom);

    setAcceptDrops(true);

    // The host is the single drop target; sections delegate every drop to it.
    for (PanelSection* section : m_sections) {
        section->setAcceptDrops(false);
    }

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_dropOverlay = new DropOverlay(this);

    rebuild();
    wireDropMoves();
}

PanelSection* SectionHost::section(const SectionSide side) const {
    return m_sections[indexOf(side)];
}

void SectionHost::setCornerOwner(const CornerOwner owner) {
    if (m_cornerOwner == owner) {
        return;
    }
    m_cornerOwner = owner;
    rebuild();
}

CornerOwner SectionHost::cornerOwner() const {
    return m_cornerOwner;
}

int SectionHost::indexOf(const SectionSide side) {
    return static_cast<int>(side);
}

bool SectionHost::ownsSection(const PanelSection* section) const {
    return std::ranges::any_of(m_sections, [section](const PanelSection* candidate) { return candidate == section; });
}

void SectionHost::rebuild() {
    const auto detach = [](QWidget* widget) {
        if (widget == nullptr) {
            return;
        }
        if (const QWidget* parentWidget = widget->parentWidget()) {
            if (QLayout* layout = parentWidget->layout()) {
                layout->removeWidget(widget);
            }
        }
        widget->setParent(nullptr);
    };

    detach(m_central);
    for (PanelSection* section : m_sections) {
        detach(section);
    }

    if (m_outer != nullptr) {
        delete m_outer; // also deletes m_inner, which is its child
        m_outer = nullptr;
        m_inner = nullptr;
    }

    const bool sidesTakeCorners = (m_cornerOwner == CornerOwner::VerticalSides);
    m_outer = new QSplitter(sidesTakeCorners ? Qt::Horizontal : Qt::Vertical, this);
    m_inner = new QSplitter(sidesTakeCorners ? Qt::Vertical : Qt::Horizontal);
    configureSplitter(m_outer);
    configureSplitter(m_inner);

    PanelSection* left = section(SectionSide::Left);
    PanelSection* right = section(SectionSide::Right);
    PanelSection* top = section(SectionSide::Top);
    PanelSection* bottom = section(SectionSide::Bottom);

    if (sidesTakeCorners) {
        m_inner->addWidget(top);
        m_inner->addWidget(m_central);
        m_inner->addWidget(bottom);
        m_inner->setStretchFactor(1, 1);

        m_outer->addWidget(left);
        m_outer->addWidget(m_inner);
        m_outer->addWidget(right);
        m_outer->setStretchFactor(1, 1);
    } else {
        m_inner->addWidget(left);
        m_inner->addWidget(m_central);
        m_inner->addWidget(right);
        m_inner->setStretchFactor(1, 1);

        m_outer->addWidget(top);
        m_outer->addWidget(m_inner);
        m_outer->addWidget(bottom);
        m_outer->setStretchFactor(1, 1);
    }

    m_layout->addWidget(m_outer);
}

void SectionHost::wireDropMoves() {
    for (PanelSection* target : m_sections) {
        connect(target, &PanelSection::panelDropReceived, this, [this, target](const PanelId& id, PanelSection* from) {
            movePanel(id, from, target);
        });
    }
}

void SectionHost::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasFormat(panelDrag::mimeType())) {
        event->acceptProposedAction();
        updateDropOverlay(nearestSide(event->position().toPoint()));
    }
}

void SectionHost::dragLeaveEvent(QDragLeaveEvent*) {
    hideDropOverlay();
}

void SectionHost::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasFormat(panelDrag::mimeType())) {
        event->acceptProposedAction();
        updateDropOverlay(nearestSide(event->position().toPoint()));
    }
}

void SectionHost::dropEvent(QDropEvent* event) {
    hideDropOverlay();
    const auto id = panelDrag::decode(event->mimeData());
    auto* from = qobject_cast<PanelSection*>(event->source());
    if (!id.has_value() || from == nullptr || !ownsSection(from)) {
        return;
    }
    movePanel(*id, from, section(nearestSide(event->position().toPoint())));
    event->acceptProposedAction();
}

void SectionHost::movePanel(const PanelId& id, PanelSection* from, PanelSection* target) {
    if (from == nullptr || target == nullptr || from == target || !ownsSection(from)) {
        return;
    }
    PanelWidget* panel = from->takePanel(id);
    if (panel == nullptr) {
        return;
    }
    target->addPanel(id, panel);
    target->openPanel(id);
}

void SectionHost::updateDropOverlay(const SectionSide side) {
    if (m_dropOverlay == nullptr) {
        return;
    }
    PanelSection* target = section(side);
    QRect area;
    if (!target->isHidden()) {
        area = QRect(target->mapTo(this, QPoint(0, 0)), target->size());
    } else {
        // Keep closed-section bands inside the space between open sections.
        const QRect available(m_central->mapTo(this, QPoint(0, 0)), m_central->size());
        const int band = std::min(48, std::min(available.width(), available.height()) / 4);
        switch (side) {
        case SectionSide::Left:
            area = QRect(available.left(), available.top(), band, available.height());
            break;
        case SectionSide::Right:
            area = QRect(available.right() - band + 1, available.top(), band, available.height());
            break;
        case SectionSide::Top:
            area = QRect(available.left(), available.top(), available.width(), band);
            break;
        case SectionSide::Bottom:
            area = QRect(available.left(), available.bottom() - band + 1, available.width(), band);
            break;
        }
    }
    m_dropOverlay->setGeometry(area);
    m_dropOverlay->raise();
    m_dropOverlay->show();
}

void SectionHost::hideDropOverlay() {
    if (m_dropOverlay != nullptr) {
        m_dropOverlay->hide();
    }
}

SectionSide SectionHost::nearestSide(const QPoint& pos) const {
    // A visible section under the cursor wins outright.
    for (const SectionSide side : {SectionSide::Left, SectionSide::Right, SectionSide::Top, SectionSide::Bottom}) {
        const PanelSection* candidate = section(side);
        if (!candidate->isHidden() && QRect(candidate->mapTo(this, QPoint(0, 0)), candidate->size()).contains(pos)) {
            return side;
        }
    }

    // Otherwise the nearest edge decides where a closed section would open.
    const QRect bounds = rect();
    const int toLeft = pos.x() - bounds.left();
    const int toRight = bounds.right() - pos.x();
    const int toTop = pos.y() - bounds.top();
    const int toBottom = bounds.bottom() - pos.y();
    const int nearest = std::min({toLeft, toRight, toTop, toBottom});
    if (nearest == toLeft) {
        return SectionSide::Left;
    }
    if (nearest == toRight) {
        return SectionSide::Right;
    }
    if (nearest == toTop) {
        return SectionSide::Top;
    }
    return SectionSide::Bottom;
}