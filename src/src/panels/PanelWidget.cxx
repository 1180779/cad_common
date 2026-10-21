/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#include <cad_common/panels/PanelWidget.hxx>

#include <utility>

PanelWidget::PanelWidget(std::string title, QWidget* parent)
    : QWidget(parent), m_title(std::move(title)) {
}

const std::string& PanelWidget::title() const {
    return m_title;
}

void PanelWidget::setTitle(std::string title) {
    m_title = std::move(title);
}

QScrollArea* PanelWidget::createScroll() {
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    return scroll;
}

QScrollArea* PanelWidget::createScroll(QWidget* widget) {
    auto* scroll = createScroll();
    scroll->setWidget(widget);
    return scroll;
}