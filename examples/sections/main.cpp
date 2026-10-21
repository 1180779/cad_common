/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QMenuBar>
#include <QSignalBlocker>
#include <QTimer>
#include <QWidget>

#include <string>
#include <utility>

#include <cad_common/Font.hxx>
#include <cad_common/PanelSection.hxx>
#include <cad_common/PanelWidget.hxx>
#include <cad_common/SectionHost.hxx>
#include <cad_common/Style.hxx>
#include <cad_common/TitleBar.hxx>

namespace {

/// @brief A minimal panel: a title plus a small amount of filler content
class DemoPanel final : public PanelWidget {
public:
    DemoPanel(std::string title, const QString& body)
        : PanelWidget(std::move(title)) {
        auto* layout = createLayout();
        layout->addWidget(new QLabel(body));
        layout->addStretch();
    }
};

/// @brief The four sections of @p host, in a fixed order
struct SectionSet {
    PanelSection* left;
    PanelSection* right;
    PanelSection* top;
    PanelSection* bottom;
};

SectionSet sectionsOf(const SectionHost* host) {
    return {
        .left = host->section(SectionSide::Left),
        .right = host->section(SectionSide::Right),
        .top = host->section(SectionSide::Top),
        .bottom = host->section(SectionSide::Bottom),
    };
}

/// @brief Keep @p action checked exactly while @p id is open in any section of
/// @p host; triggering opens/closes the panel wherever it is registered so
/// drag-and-drop moves stay in sync
void wirePanelToggle(QAction* action, const SectionHost* host, const PanelId& id) {
    const SectionSet sections = sectionsOf(host);

    const auto isOpenAnywhere = [sections, id] {
        return sections.left->isOpen(id) || sections.right->isOpen(id)
               || sections.top->isOpen(id) || sections.bottom->isOpen(id);
    };

    QObject::connect(action, &QAction::triggered, host, [sections, id](const bool checked) {
        if (checked) {
            for (PanelSection* section : {sections.left, sections.right, sections.top, sections.bottom}) {
                if (section->hasPanel(id)) {
                    section->openPanel(id);
                    return;
                }
            }
        } else {
            for (PanelSection* section : {sections.left, sections.right, sections.top, sections.bottom}) {
                if (section->isOpen(id)) {
                    section->closePanel(id);
                }
            }
        }
    });

    const auto refresh = [action, isOpenAnywhere] {
        const bool open = isOpenAnywhere();
        if (action->isChecked() == open) {
            return;
        }
        const QSignalBlocker blocker(action);
        action->setChecked(open);
    };

    for (PanelSection* section : {sections.left, sections.right, sections.top, sections.bottom}) {
        QObject::connect(section, &PanelSection::panelOpened, action, [refresh](const PanelId&) { refresh(); });
        QObject::connect(section, &PanelSection::panelClosed, action, [refresh](const PanelId&) { refresh(); });
    }
    refresh();
}

QAction* addPanelAction(QMenu* menu, const QString& text) {
    QAction* action = menu->addAction(text);
    action->setCheckable(true);
    return action;
}

} // namespace

int main(int argc, char* argv[]) {
    const QApplication app(argc, argv);
    const QStringList args = QApplication::arguments();
    const bool forceDark = args.contains(QStringLiteral("--dark"));
    cad_common::style::apply(forceDark ? cad_common::StyleMode::Dark : cad_common::StyleMode::System);
    [[maybe_unused]] const QString appFontFamily = cad_common::applyAppFont();

    auto* central = new QFrame;
    central->setObjectName(QStringLiteral("centralViewport"));
    central->setFrameShape(QFrame::NoFrame);
    central->setAttribute(Qt::WA_StyledBackground, true);
    central->setMinimumSize(360, 280);
    auto* centralLayout = new QVBoxLayout(central);
    centralLayout->addWidget(new QLabel(QStringLiteral("Central widget"), central));
    centralLayout->addStretch();

    auto* host = new SectionHost(central);
    host->setCornerOwner(CornerOwner::HorizontalSides);

    PanelSection* left = host->section(SectionSide::Left);
    PanelSection* right = host->section(SectionSide::Right);
    PanelSection* top = host->section(SectionSide::Top);
    PanelSection* bottom = host->section(SectionSide::Bottom);

    left->setMode(SectionMode::Split);
    left->setMaximumOpenPanels(2);
    right->setMode(SectionMode::Exclusive);
    top->setMode(SectionMode::Exclusive);
    bottom->setMode(SectionMode::Exclusive);

    const PanelId parametersId("parameters");
    const PanelId controlsId("controls");
    const PanelId plotsId("plots");
    const PanelId forcesId("forces");
    const PanelId stateId("state");

    left->addPanel(parametersId, new DemoPanel("Parameters", "m, c, k controls"));
    left->addPanel(controlsId, new DemoPanel("Controls", "x(0), v(0), integration step"));
    right->addPanel(plotsId, new DemoPanel("Plots", "force and trajectory plots"));
    top->addPanel(forcesId, new DemoPanel("Forces", "f(t), g(t), h(t)"));
    bottom->addPanel(stateId, new DemoPanel("State", "x(t), v(t), a(t)"));

    auto* window = new QWidget;
    window->setWindowFlag(Qt::FramelessWindowHint);
    window->setAutoFillBackground(true);
    window->setAcceptDrops(true); // swallow drops over the chrome so Qt does not warn

    constexpr int resizeMargin = 6;
    auto* outerLayout = new QVBoxLayout(window);
    outerLayout->setContentsMargins(resizeMargin, resizeMargin, resizeMargin, resizeMargin);
    outerLayout->setSpacing(resizeMargin);

    auto* titleBar = new TitleBar(window);
    QMenuBar* menuBar = titleBar->menuBar();

    QMenu* panelsMenu = menuBar->addMenu(QStringLiteral("Panels"));
    QAction* parametersAction = addPanelAction(panelsMenu, QStringLiteral("Parameters"));
    QAction* controlsAction = addPanelAction(panelsMenu, QStringLiteral("Controls"));
    QAction* plotsAction = addPanelAction(panelsMenu, QStringLiteral("Plots"));
    QAction* forcesAction = addPanelAction(panelsMenu, QStringLiteral("Forces"));
    QAction* stateAction = addPanelAction(panelsMenu, QStringLiteral("State"));

    QMenu* viewMenu = menuBar->addMenu(QStringLiteral("View"));
    QAction* splitAction = viewMenu->addAction(QStringLiteral("Left section split"));
    splitAction->setCheckable(true);
    splitAction->setChecked(true);
    QAction* darkAction = viewMenu->addAction(QStringLiteral("Dark mode"));
    darkAction->setCheckable(true);
    darkAction->setChecked(forceDark || QApplication::palette().color(QPalette::Window).lightness() < 128);

    QMenu* fileMenu = menuBar->addMenu(QStringLiteral("File"));
    const QAction* quitAction = fileMenu->addAction(QStringLiteral("Quit"));

    outerLayout->addWidget(titleBar);
    outerLayout->addWidget(host, 1);

    enableFramelessResize(window, resizeMargin);

    wirePanelToggle(parametersAction, host, parametersId);
    wirePanelToggle(controlsAction, host, controlsId);
    wirePanelToggle(plotsAction, host, plotsId);
    wirePanelToggle(forcesAction, host, forcesId);
    wirePanelToggle(stateAction, host, stateId);

    QObject::connect(splitAction, &QAction::triggered, window, [left](const bool split) {
        left->setMode(split ? SectionMode::Split : SectionMode::Exclusive);
    });

    QObject::connect(darkAction, &QAction::triggered, window, [](const bool dark) {
        cad_common::style::apply(dark ? cad_common::StyleMode::Dark : cad_common::StyleMode::Light);
    });

    QObject::connect(quitAction, &QAction::triggered, window, [window] { window->close(); });

    left->openPanel(parametersId);
    right->openPanel(plotsId);

    if (args.contains(QStringLiteral("--self-test"))) {
        bool ok = true;
        const auto check = [&ok](const bool condition, const char* what) {
            if (condition) {
                qInfo("ok: %s", what);
            } else {
                ok = false;
                qWarning("FAIL: %s", what);
            }
        };

        check(host->acceptDrops(), "section host accepts drops (removes drop warning)");
        check(!appFontFamily.isEmpty(), "JetBrains Mono font loaded");
        check(parametersAction->isChecked(), "parameters action checked initially");
        check(plotsAction->isChecked(), "plots action checked initially");
        check(!controlsAction->isChecked(), "controls action unchecked initially");
        check(left->isOpen(parametersId), "parameters panel open initially");
        check(right->isOpen(plotsId), "plots panel open initially");

        controlsAction->trigger();
        check(controlsAction->isChecked(), "controls action checked after opening");
        check(left->isOpen(controlsId), "controls panel open after opening");

        controlsAction->trigger();
        check(!controlsAction->isChecked(), "controls action unchecked after closing");
        check(!left->isOpen(controlsId), "controls panel closed after closing");

        parametersAction->trigger();
        check(!parametersAction->isChecked(), "parameters action unchecked after closing");
        check(!left->isOpen(parametersId), "parameters panel closed after closing");

        // Simulate a drag move: take the panel out of the left section and open
        // it in the empty top section; the action must follow the new owner.
        PanelWidget* moved = left->takePanel(parametersId);
        top->addPanel(parametersId, moved);
        top->openPanel(parametersId);
        check(parametersAction->isChecked(), "parameters action checked after moving to top");
        check(!left->isOpen(parametersId), "parameters no longer open in left after move");
        check(top->isOpen(parametersId), "parameters open in top after move");

        parametersAction->trigger();
        check(!parametersAction->isChecked(), "parameters action unchecked after closing moved panel");
        check(!top->isOpen(parametersId), "parameters closed in new owner after click");

        qInfo("self-test %s", ok ? "PASSED" : "FAILED");
        return ok ? 0 : 1;
    }

    window->resize(1100, 700);
    window->show();

    return QApplication::exec();
}