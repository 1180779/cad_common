/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_COORDINATE_SPIN_BOX_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_COORDINATE_SPIN_BOX_HXX

#include <limits>

#include <QBoxLayout>
#include <QLabel>

#include "ModifierSpinBox.hxx"

/// @brief Shared configuration for coordinate spinboxes
namespace coordSpinBox {
inline constexpr double gc_min = std::numeric_limits<double>::lowest();
inline constexpr double gc_max = std::numeric_limits<double>::max();
inline constexpr double gc_step = 0.1;
inline constexpr int gc_decimals = 4;
inline constexpr int gc_width = 100;

/// @brief Apply the shared coordinate-spinbox settings to @p sb
inline void configure(ModifierDoubleSpinBox* sb) {
    sb->setRange(gc_min, gc_max);
    sb->setSingleStep(gc_step);
    sb->setDecimals(gc_decimals);
    sb->setFixedWidth(gc_width);
    sb->setKeyboardTracking(true);
    sb->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
}

/// @brief Append a @p label and a configured coordinate spinbox to @p layout,
/// returning the spinbox. Gives every point editor an identical labeled row
inline ModifierDoubleSpinBox* addRow(QBoxLayout* layout, const QString& label) {
    layout->addWidget(new QLabel(label));
    auto* sb = new ModifierDoubleSpinBox();
    configure(sb);
    layout->addWidget(sb);
    return sb;
}

/// @brief Build the standard X/Y/Z labeled rows into @p layout
inline void setUpRows(
    QBoxLayout* layout,
    ModifierDoubleSpinBox*& x,
    ModifierDoubleSpinBox*& y,
    ModifierDoubleSpinBox*& z
    ) {
    x = addRow(layout, "X:");
    y = addRow(layout, "Y:");
    z = addRow(layout, "Z:");
}
}


#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_COORDINATE_SPIN_BOX_HXX