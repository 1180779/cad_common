/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#include <QApplication>
#include <QBoxLayout>
#include <QDebug>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <string>
#include <utility>

#include <cad_common/PanelSection.hxx>
#include <cad_common/PanelWidget.hxx>
#include <cad_common/SectionHost.hxx>
#include <cad_common/Style.hxx>

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

QPushButton* addToggle(QBoxLayout* layout, const QString& text) {
    auto* button = new QPushButton(text);
    button->setCheckable(true);
    layout->addWidget(button);
    return button;
}

/// @brief The four sections of @p host, in a fixed order
struct SectionSet {
    PanelSection* left;
    PanelSection* right;
    PanelSection* top;
    PanelSection* bottom;
};

SectionSet sectionsOf(SectionHost* host) {
    return {
        host->section(SectionSide::Left),
        host->section(SectionSide::Right),
        host->section(SectionSide::Top),
        host->section(SectionSide::Bottom),
    };
}

/// @brief Keep @p button checked exactly while @p id is open in any section of
/// @p host; clicking opens/closes the panel wherever it is currently registered
/// so drag-and-drop moves stay in sync
void wirePanelToggle(QPushButton* button, SectionHost* host, const PanelId& id) {
    const SectionSet sections = sectionsOf(host);

    const auto isOpenAnywhere = [sections, id] {
        return sections.left->isOpen(id) || sections.right->isOpen(id)
               || sections.top->isOpen(id) || sections.bottom->isOpen(id);
    };

    QObject::connect(button, &QPushButton::clicked, host, [sections, id](const bool checked) {
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

    const auto refresh = [button, isOpenAnywhere] {
        const bool open = isOpenAnywhere();
        if (button->isChecked() == open) {
            return;
        }
        const QSignalBlocker blocker(button);
        button->setChecked(open);
    };

    for (PanelSection* section : {sections.left, sections.right, sections.top, sections.bottom}) {
        QObject::connect(section, &PanelSection::panelOpened, button, [refresh](PanelId) { refresh(); });
        QObject::connect(section, &PanelSection::panelClosed, button, [refresh](PanelId) { refresh(); });
    }
    refresh();
}

} // namespace

int main(int argc, char* argv[]) {
    const QApplication app(argc, argv);
    const QStringList args = QApplication::arguments();
    const bool forceDark = args.contains(QStringLiteral("--dark"));
    cad_common::style::apply(forceDark ? cad_common::StyleMode::Dark : cad_common::StyleMode::System);

    auto* central = new QFrame;
    central->setObjectName(QStringLiteral("centralViewport"));
    central->setFrameShape(QFrame::NoFrame);
    central->setAttribute(Qt::WA_StyledBackground, true);
    central->setMinimumSize(360, 280);
    auto* centralLayout = new QVBoxLayout(central);
    centralLayout->addWidget(new QLabel(QStringLiteral("Central widget"), central));
    centralLayout->addStretch();

    auto* host = new SectionHost(central);

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
    window->setAcceptDrops(true); // swallow drops over the toolbar so Qt does not warn
    auto* windowLayout = new QVBoxLayout(window);
    auto* toolbar = new QWidget(window);
    auto* toolbarLayout = new QHBoxLayout(toolbar);

    auto* parametersBtn = addToggle(toolbarLayout, "Parameters");
    auto* controlsBtn = addToggle(toolbarLayout, "Controls");
    auto* plotsBtn = addToggle(toolbarLayout, "Plots");
    auto* forcesBtn = addToggle(toolbarLayout, "Forces");
    auto* stateBtn = addToggle(toolbarLayout, "State");

    auto* cornersBtn = new QPushButton(QStringLiteral("Corners: top/bottom"), toolbar);
    toolbarLayout->addWidget(cornersBtn);
    auto* leftModeBtn = new QPushButton(QStringLiteral("Left: Split"), toolbar);
    toolbarLayout->addWidget(leftModeBtn);
    auto* styleBtn = new QPushButton(QStringLiteral("Style: Dark"), toolbar);
    toolbarLayout->addWidget(styleBtn);

    windowLayout->addWidget(toolbar);
    windowLayout->addWidget(host, 1);

    wirePanelToggle(parametersBtn, host, parametersId);
    wirePanelToggle(controlsBtn, host, controlsId);
    wirePanelToggle(plotsBtn, host, plotsId);
    wirePanelToggle(forcesBtn, host, forcesId);
    wirePanelToggle(stateBtn, host, stateId);

    QObject::connect(cornersBtn, &QPushButton::clicked, window, [host, cornersBtn] {
        const bool sides = host->cornerOwner() == CornerOwner::VerticalSides;
        host->setCornerOwner(sides ? CornerOwner::HorizontalSides : CornerOwner::VerticalSides);
        cornersBtn->setText(sides ? QStringLiteral("Corners: left/right") : QStringLiteral("Corners: top/bottom"));
    });

    QObject::connect(leftModeBtn, &QPushButton::clicked, window, [left, leftModeBtn] {
        const bool split = left->mode() == SectionMode::Split;
        left->setMode(split ? SectionMode::Exclusive : SectionMode::Split);
        leftModeBtn->setText(split ? QStringLiteral("Left: Split") : QStringLiteral("Left: Exclusive"));
    });

    QObject::connect(styleBtn, &QPushButton::clicked, window, [styleBtn] {
        const bool toDark = styleBtn->text() == QStringLiteral("Style: Dark");
        cad_common::style::apply(toDark ? cad_common::StyleMode::Dark : cad_common::StyleMode::Light);
        styleBtn->setText(toDark ? QStringLiteral("Style: Light") : QStringLiteral("Style: Dark"));
    });

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
        check(parametersBtn->isChecked(), "parameters button checked initially");
        check(plotsBtn->isChecked(), "plots button checked initially");
        check(!controlsBtn->isChecked(), "controls button unchecked initially");
        check(left->isOpen(parametersId), "parameters panel open initially");
        check(right->isOpen(plotsId), "plots panel open initially");

        controlsBtn->click();
        check(controlsBtn->isChecked(), "controls button checked after opening");
        check(left->isOpen(controlsId), "controls panel open after opening");

        controlsBtn->click();
        check(!controlsBtn->isChecked(), "controls button unchecked after closing");
        check(!left->isOpen(controlsId), "controls panel closed after closing");

        parametersBtn->click();
        check(!parametersBtn->isChecked(), "parameters button unchecked after closing");
        check(!left->isOpen(parametersId), "parameters panel closed after closing");

        // Simulate a drag move: take the panel out of the left section and open
        // it in the empty top section; the button must follow the new owner.
        PanelWidget* moved = left->takePanel(parametersId);
        top->addPanel(parametersId, moved);
        top->openPanel(parametersId);
        check(parametersBtn->isChecked(), "parameters button checked after moving to top");
        check(!left->isOpen(parametersId), "parameters no longer open in left after move");
        check(top->isOpen(parametersId), "parameters open in top after move");

        parametersBtn->click();
        check(!parametersBtn->isChecked(), "parameters button unchecked after closing moved panel");
        check(!top->isOpen(parametersId), "parameters closed in new owner after click");

        qInfo("self-test %s", ok ? "PASSED" : "FAILED");
        return ok ? 0 : 1;
    }

    window->resize(1100, 700);
    window->show();

    if (const int idx = static_cast<int>(args.indexOf(QStringLiteral("--screenshot"))); idx != -1) {
        const QString path = idx + 1 < args.size() ? args[idx + 1] : QStringLiteral("screenshot.png");
        QTimer::singleShot(0, &app, [&window, path] {
            static_cast<void>(window->grab().save(path));
            QApplication::quit();
        });
    }

    return QApplication::exec();
}