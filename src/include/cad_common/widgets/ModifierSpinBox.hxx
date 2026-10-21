/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_MODIFIER_SPIN_BOX_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_MODIFIER_SPIN_BOX_HXX

#include <QAbstractButton>
#include <QApplication>
#include <QLineEdit>
#include <QShowEvent>
#include <QSpinBox>

/// Spinboxes that scale steps with modifiers
class ModifierDoubleSpinBox final : public QDoubleSpinBox {
    Q_OBJECT

public:
    explicit ModifierDoubleSpinBox(QWidget* parent = nullptr) : QDoubleSpinBox(parent) {
        lineEdit()->setMinimumWidth(50);
    }

    void stepBy(int steps) override {
        if (const auto mods = QApplication::keyboardModifiers();
            mods & Qt::ShiftModifier) {
            steps *= 100;
        }
        QDoubleSpinBox::stepBy(steps);
    }

protected:
    void showEvent(QShowEvent* e) override {
        QDoubleSpinBox::showEvent(e);
        for (auto* btn : findChildren<QAbstractButton*>()) {
            btn->setMaximumWidth(16);
        }
    }
};

class ModifierSpinBox final : public QSpinBox {
    Q_OBJECT

public:
    explicit ModifierSpinBox(QWidget* parent = nullptr) : QSpinBox(parent) {
        lineEdit()->setMinimumWidth(50);
    }

    void stepBy(int steps) override {
        if (const auto mods = QApplication::keyboardModifiers();
            mods & Qt::ShiftModifier) {
            steps *= 100;
        }
        QSpinBox::stepBy(steps);
    }

protected:
    void showEvent(QShowEvent* e) override {
        QSpinBox::showEvent(e);
        for (auto* btn : findChildren<QAbstractButton*>()) {
            btn->setMaximumWidth(16);
        }
    }
};

#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_MODIFIER_SPIN_BOX_HXX