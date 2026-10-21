/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#include <cad_common/panels/PanelSection.hxx>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <ranges>
#include <utility>

#include <QApplication>
#include <QCursor>
#include <QDataStream>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QIODevice>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPoint>
#include <QPixmap>
#include <QSplitter>
#include <QToolButton>
#include <QVBoxLayout>

#include <cad_common/panels/PanelDrag.hxx>
#include <cad_common/panels/PanelWidget.hxx>

namespace {

/// @brief Header row (title + close button) that also acts as a drag handle
class SlotHeader final : public QFrame {
public:
    explicit SlotHeader(QWidget* parent = nullptr)
        : QFrame(parent) {
        setObjectName(QStringLiteral("panelHeader"));
        setAttribute(Qt::WA_StyledBackground, true);
        setFrameShape(NoFrame);
        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(6, 2, 2, 2);
        m_title = new QLabel(this);

        auto* close = new QToolButton(this);
        close->setObjectName(QStringLiteral("panelCloseButton"));
        close->setText(QStringLiteral("\u00d7"));
        close->setAutoRaise(true);
        close->setCursor(Qt::ArrowCursor);
        connect(close, &QToolButton::clicked, this, [this] {
            if (onCloseRequested) {
                onCloseRequested();
            }
        });

        layout->addWidget(m_title);
        layout->addStretch();
        layout->addWidget(close);

        setCursor(Qt::OpenHandCursor);
    }

    void setTitle(const QString& title) const {
        m_title->setText(title);
    }

    std::function<void()> onCloseRequested;
    std::function<void()> onDragRequested;

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            m_dragStart = event->pos();
            event->accept();
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if ((event->buttons() & Qt::LeftButton)
            && (event->pos() - m_dragStart).manhattanLength() >= QApplication::startDragDistance()) {
            if (onDragRequested) {
                onDragRequested();
            }
        }
    }

private:
    QLabel* m_title;
    QPoint m_dragStart;
};
} // namespace

class PanelSection::Slot final : public QFrame {
public:
    explicit Slot(QWidget* parent = nullptr)
        : QFrame(parent) {
        setObjectName(QStringLiteral("panelSlot"));
        setAttribute(Qt::WA_StyledBackground, true);
        setFrameShape(NoFrame);
        m_header = new SlotHeader(this);
        m_contentLayout = new QVBoxLayout;
        m_contentLayout->setContentsMargins(0, 0, 0, 0);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->addWidget(m_header);
        layout->addLayout(m_contentLayout, 1);
    }

    void setContent(PanelWidget* widget, const PanelId& id) {
        if (m_content != nullptr) {
            m_contentLayout->removeWidget(m_content);
            m_content->setParent(nullptr);
        }
        m_content = widget;
        m_id = id;
        if (m_content != nullptr) {
            m_contentLayout->addWidget(m_content);
            m_header->setTitle(QString::fromStdString(m_content->title()));
        } else {
            m_header->setTitle(QString());
        }
    }

    [[nodiscard]] PanelWidget* content() const {
        return m_content;
    }

    [[nodiscard]] const PanelId& id() const {
        return m_id;
    }

    [[nodiscard]] SlotHeader* header() const {
        return m_header;
    }

private:
    SlotHeader* m_header;
    QVBoxLayout* m_contentLayout;
    PanelWidget* m_content = nullptr;
    PanelId m_id;
};

PanelSection::PanelSection(const SectionSide side, QWidget* parent)
    : QWidget(parent), m_side(side) {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    setAcceptDrops(true);
    rebuild();
}

PanelSection::~PanelSection() {
    for (Slot* slot : m_slots) {
        if (slot != nullptr) {
            slot->setContent(nullptr, PanelId());
        }
    }
}

void PanelSection::setMode(const SectionMode mode) {
    if (m_mode == mode) {
        return;
    }
    m_mode = mode;
    rebuild();
}

SectionMode PanelSection::mode() const {
    return m_mode;
}

void PanelSection::setMaximumOpenPanels(const int count) {
    const int clamped = std::max(1, count);
    if (m_maximumOpenPanels == clamped) {
        return;
    }
    m_maximumOpenPanels = clamped;
    rebuild();
}

int PanelSection::maximumOpenPanels() const {
    return m_maximumOpenPanels;
}

bool PanelSection::addPanel(PanelId id, PanelWidget* panel) {
    if (panel == nullptr || m_panels.contains(id)) {
        return false;
    }
    m_panels.emplace(std::move(id), panel);
    return true;
}

PanelWidget* PanelSection::takePanel(const PanelId& id) {
    const auto it = m_panels.find(id);
    if (it == m_panels.end()) {
        return nullptr;
    }
    PanelWidget* panel = it->second;
    closePanel(id);
    m_panels.erase(it);
    return panel;
}

void PanelSection::openPanel(const PanelId& id) {
    const auto it = m_panels.find(id);
    if (it == m_panels.end()) {
        return;
    }
    if (m_open.contains(id)) {
        if (const Slot* slot = slotFor(id); slot != nullptr && slot->content() != nullptr) {
            slot->content()->setFocus(Qt::OtherFocusReason);
        }
        return;
    }

    Slot* slot = firstEmptySlot();
    if (slot == nullptr) {
        for (const auto& mSlot : std::views::reverse(m_slots)) {
            if (mSlot->content() != nullptr) {
                slot = mSlot;
                break;
            }
        }
        if (slot == nullptr) {
            return;
        }
        closePanel(slot->id());
    }

    slot->setContent(it->second, id);
    slot->show();
    m_open.emplace(id, slot);
    updateVisibility();
    emit panelOpened(id);
}

void PanelSection::closePanel(const PanelId& id) {
    const auto it = m_open.find(id);
    if (it == m_open.end()) {
        return;
    }
    Slot* slot = it->second;
    m_open.erase(it);
    slot->setContent(nullptr, PanelId());
    slot->hide();
    updateVisibility();
    emit panelClosed(id);
}

void PanelSection::togglePanel(const PanelId& id) {
    if (isOpen(id)) {
        closePanel(id);
    } else {
        openPanel(id);
    }
}

bool PanelSection::isOpen(const PanelId& id) const {
    return m_open.contains(id);
}

bool PanelSection::hasPanel(const PanelId& id) const {
    return m_panels.contains(id);
}

void PanelSection::rebuild() {
    std::vector<PanelId> openIds;
    openIds.reserve(m_open.size());
    for (const auto& [id, slot] : m_open) {
        slot->setContent(nullptr, PanelId());
        openIds.push_back(id);
    }
    m_open.clear();

    if (m_splitter != nullptr) {
        m_layout->removeWidget(m_splitter);
        delete m_splitter;
        m_splitter = nullptr;
    }
    m_slots.clear();

    const int slotCount = (m_mode == SectionMode::Exclusive) ? 1 : m_maximumOpenPanels;
    const bool vertical = (m_side == SectionSide::Left || m_side == SectionSide::Right);
    m_splitter = new QSplitter(vertical ? Qt::Vertical : Qt::Horizontal);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setHandleWidth(4);
    m_layout->addWidget(m_splitter);

    for (int i = 0; i < slotCount; ++i) {
        auto* slot = new Slot(m_splitter);
        slot->hide();
        m_slots.push_back(slot);
        m_splitter->addWidget(slot);

        slot->header()->onCloseRequested = [this, slot] {
            if (slot->content() != nullptr) {
                closePanel(slot->id());
            }
        };

        slot->header()->onDragRequested = [this, slot] {
            if (slot->content() == nullptr) {
                return;
            }
            auto* mime = new QMimeData;
            mime->setData(panelDrag::mimeType(), panelDrag::encode(slot->id()));

            QPixmap pixmap = slot->grab();
            if (pixmap.width() > 240 || pixmap.height() > 240) {
                pixmap = pixmap.scaled(240, 240, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }

            QDrag drag(this);
            drag.setMimeData(mime);
            drag.setPixmap(pixmap);
            drag.setHotSpot(slot->mapFromGlobal(QCursor::pos()));
            drag.exec(Qt::MoveAction);
        };
    }

    for (const auto& id : openIds) {
        placeOpenPanel(id);
    }
    updateVisibility();
}

void PanelSection::placeOpenPanel(const PanelId& id) {
    const auto it = m_panels.find(id);
    if (it == m_panels.end()) {
        return;
    }
    Slot* slot = firstEmptySlot();
    if (slot == nullptr) {
        return;
    }
    slot->setContent(it->second, id);
    slot->show();
    m_open.emplace(id, slot);
}

void PanelSection::updateVisibility() {
    if (m_open.empty()) {
        hide();
    } else {
        show();
    }
}

PanelSection::Slot* PanelSection::firstEmptySlot() const {
    for (Slot* slot : m_slots) {
        if (slot->content() == nullptr) {
            return slot;
        }
    }
    return nullptr;
}

PanelSection::Slot* PanelSection::slotFor(const PanelId& id) const {
    const auto it = m_open.find(id);
    return it == m_open.end() ? nullptr : it->second;
}

void PanelSection::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasFormat(panelDrag::mimeType())) {
        event->acceptProposedAction();
    }
}

void PanelSection::dragMoveEvent(QDragMoveEvent* event) {
    if (event->mimeData()->hasFormat(panelDrag::mimeType())) {
        event->acceptProposedAction();
    }
}

void PanelSection::dropEvent(QDropEvent* event) {
    const auto id = panelDrag::decode(event->mimeData());
    auto* from = qobject_cast<PanelSection*>(event->source());
    if (id.has_value()) {
        emit panelDropReceived(*id, from);
        event->acceptProposedAction();
    }
}