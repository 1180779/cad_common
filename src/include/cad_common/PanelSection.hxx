/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_PANEL_SECTION_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_PANEL_SECTION_HXX

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <QWidget>

#include "PanelId.hxx"

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QSplitter;
class QVBoxLayout;
class PanelWidget;

/// @brief Side of the host a section is docked on
enum class SectionSide { Left, Right, Top, Bottom };

/// @brief Whether the section may show several panels at once
enum class SectionMode {
    Split, ///< Up to <tt>maximumOpenPanels()</tt> panels share the section via a splitter
    Exclusive, ///< One panel visible at a time; fills the whole section
};

/// @brief A dockable, collapsible, resizable stack of panels on one side
///
/// The section hides itself while no panel is open. Each panel's header row
/// doubles as a drag handle
///
/// The section never deletes panels: the caller keeps ownership of every
/// PanelWidget, and takePanel() simply detaches and unregisters one
class PanelSection final : public QWidget {
    Q_OBJECT

public:
    explicit PanelSection(SectionSide side, QWidget* parent = nullptr);
    ~PanelSection() override;

    void setMode(SectionMode mode);
    [[nodiscard]] SectionMode mode() const;

    /// @brief Number of simultaneously visible panels in <tt>Split</tt> mode
    /// (>= 1)
    void setMaximumOpenPanels(int count);
    [[nodiscard]] int maximumOpenPanels() const;

    /// @brief Register @p panel under @p id; does not show it
    /// @returns false if @p id is already registered
    bool addPanel(PanelId id, PanelWidget* panel);

    /// @brief Detach and unregister the panel under @p id, closing it first if
    /// open. Returns the panel (still caller-owned) or <tt>nullptr</tt>
    [[nodiscard]] PanelWidget* takePanel(const PanelId& id);

    void openPanel(const PanelId& id);
    void closePanel(const PanelId& id);
    void togglePanel(const PanelId& id);
    [[nodiscard]] bool isOpen(const PanelId& id) const;

    /// @brief Whether @p id is registered in this section (open or closed)
    [[nodiscard]] bool hasPanel(const PanelId& id) const;

signals:
    void panelOpened(PanelId id);
    void panelClosed(PanelId id);

    /// @brief A panel dragged from @p from was dropped on this section. The
    /// owner performs the move via takePanel/addPanel
    void panelDropReceived(PanelId id, PanelSection* from);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    class Slot;

    void rebuild();
    void placeOpenPanel(const PanelId& id);
    void updateVisibility();
    [[nodiscard]] Slot* firstEmptySlot() const;
    [[nodiscard]] Slot* slotFor(const PanelId& id) const;

    SectionSide m_side;
    SectionMode m_mode = SectionMode::Exclusive;
    int m_maximumOpenPanels = 2;

    QVBoxLayout* m_layout = nullptr;
    QSplitter* m_splitter = nullptr;
    std::vector<Slot*> m_slots;

    std::unordered_map<PanelId, PanelWidget*> m_panels;
    std::unordered_map<PanelId, Slot*> m_open;
};

#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_PANEL_SECTION_HXX