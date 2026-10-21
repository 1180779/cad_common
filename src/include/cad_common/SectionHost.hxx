/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_SECTION_HOST_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_SECTION_HOST_HXX

#include <array>

#include <QPoint>
#include <QWidget>

#include "PanelSection.hxx"

class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QSplitter;
class QVBoxLayout;
class PanelSection;

/// @brief Which pair of sections spans the full extent (takes the corners)
enum class CornerOwner {
    VerticalSides, ///< Left/right span full height
    HorizontalSides, ///< Top/bottom span full width
};

/// @brief A central widget surrounded by up to four PanelSection instances
///
/// The host owns the splitters that make the sections resizable against the
/// central widget, and routes drag-and-drop moves between its own sections
class SectionHost final : public QWidget {
    Q_OBJECT

public:
    /// @brief @p central is reparented into the host (ownership transfers)
    explicit SectionHost(QWidget* central, QWidget* parent = nullptr);

    /// @brief The section docked on @p side (always non-null)
    [[nodiscard]] PanelSection* section(SectionSide side) const;

    void setCornerOwner(CornerOwner owner);
    [[nodiscard]] CornerOwner cornerOwner() const;

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    class DropOverlay;

    void rebuild();
    void wireDropMoves();
    void movePanel(const PanelId& id, PanelSection* from, PanelSection* target);
    void updateDropOverlay(SectionSide side);
    void hideDropOverlay();
    [[nodiscard]] SectionSide nearestSide(const QPoint& pos) const;
    [[nodiscard]] bool ownsSection(const PanelSection* section) const;
    [[nodiscard]] static int indexOf(SectionSide side);

    QWidget* m_central;
    CornerOwner m_cornerOwner = CornerOwner::VerticalSides;
    std::array<PanelSection*, 4> m_sections{};
    QVBoxLayout* m_layout = nullptr;
    QSplitter* m_outer = nullptr;
    QSplitter* m_inner = nullptr;
    DropOverlay* m_dropOverlay = nullptr;
};

#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_SECTION_HOST_HXX