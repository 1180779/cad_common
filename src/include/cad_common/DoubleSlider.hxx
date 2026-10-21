/*
 * Created by Radosław Głasek on 21.10.2026
 * radoslaw.glasek@theworldlooksback.dev
 */

#ifndef CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_DOUBLE_SLIDER_HXX
#define CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_DOUBLE_SLIDER_HXX

#include <QSlider>
#include <QObject>

//
// inspired by stack overflow
// https://stackoverflow.com/questions/19003369/how-to-make-a-qslider-change-with-double-values
//

class DoubleSlider : public QSlider {
    Q_OBJECT

public:
    explicit DoubleSlider(QWidget* parent = nullptr);

    void setMappingRange(double start, double end);

    void setValue(double value);

    double value() const;

signals:
    void doubleValueChanged(double value);

public
slots:
    void notifyValueChanged(int value);

private:
    double m_mappingRangeStart = 0.0;
    double m_mappingRangeEnd = 1.0;
    double m_mappingRangeLength = 1.0;
};

#endif // CAD_COMMON_HEADER_SRC_INCLUDE_CAD_COMMON_DOUBLE_SLIDER_HXX