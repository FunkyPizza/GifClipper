#include "QSimpleTimeline.h"
#include "QStyleManager.h"
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionComplex>

QSimpleTimeline::QSimpleTimeline(QWidget *parent)
    : QWidget(parent)
{
    m_lowValue = m_minimum;
    m_highValue = m_maximum;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    updateColorsFromStylesheet();
}

QSimpleTimeline::~QSimpleTimeline() {}

unsigned int QSimpleTimeline::getRangeMin() const
{
    return m_minimum;
}

void QSimpleTimeline::setRangeMin(unsigned int const minimum)
{
    if (m_minimum != minimum)
    {
        m_minimum = minimum;
        if (m_minimum >= m_maximum)
        {
            setRangeMax(m_minimum + 1);
            setRangeHigh(m_maximum);
            setRangeLow(m_minimum);
        }
        else if (m_minimum >= m_highValue)
        {
            setRangeHigh(m_minimum + 1);
            setRangeLow(m_minimum);
        }
        else if (m_minimum > m_lowValue)
        {
            setRangeLow(m_minimum);
        }

        update();
        emit(minimumChanged(m_minimum));
    }
}

unsigned int QSimpleTimeline::getRangeMax() const
{
    return m_maximum;
}

void QSimpleTimeline::setRangeMax(unsigned int const maximum)
{
    if (maximum != m_maximum)
    {
        m_maximum = maximum;
        if (m_maximum <= m_minimum)
        {
            setRangeMin(m_maximum++);
            setRangeHigh(m_maximum);
            setRangeLow(m_minimum);
        }
        else if (m_maximum <= m_lowValue)
        {
            setRangeHigh(m_maximum);
            setRangeLow(m_highValue - 1);
        }
        else if (m_maximum < m_highValue)
        {
            setRangeHigh(m_maximum);
        }

        update();
        emit(maximumChanged(m_maximum));
    }
}

unsigned int QSimpleTimeline::getRangeLow() const
{
    return m_lowValue;
}

void QSimpleTimeline::setRangeLow(unsigned int const lowValue)
{
    if (m_lowValue != lowValue)
    {
        m_lowValue = std::clamp(lowValue, m_minimum, m_maximum - MINIMUM_RANGE);

        if (m_lowValue >= m_highValue - MINIMUM_RANGE) {
            setRangeHigh(m_lowValue + MINIMUM_RANGE);
        }
        if (m_lowValue >= m_highValue) {
            setRangeHigh(fmax(m_highValue, m_lowValue + MINIMUM_RANGE));
        }

        update();
        emit(lowValueChanged(m_lowValue));
        emit(rangeChanged(m_lowValue, m_highValue));
    }
}

unsigned int QSimpleTimeline::getRangeHigh() const
{
    return m_highValue;
}

unsigned int QSimpleTimeline::getCursorValue() const
{
    return m_CursorValue;
}

void QSimpleTimeline::setRangeHigh(unsigned int const highValue)
{
    if (m_highValue != highValue)
    {
        m_highValue = std::clamp(highValue, m_minimum + MINIMUM_RANGE, m_maximum);

        if (m_highValue <= m_lowValue + MINIMUM_RANGE) {
            setRangeLow(m_highValue - MINIMUM_RANGE);
        }
        if (m_highValue <= m_lowValue) {
            setRangeLow(fmin(m_lowValue, m_highValue - MINIMUM_RANGE));
        }

        update();
        emit(highValueChanged(m_highValue));
        emit(rangeChanged(m_lowValue, m_highValue));
    }
}

void QSimpleTimeline::setCursorValue(unsigned int const Value)
{
    m_CursorValue = std::clamp(Value, m_minimum, m_maximum);
    update();

    if (m_pressedElement == QTimelineElements::playerHandle) {
        emit cursorValueChanged(Value);
    }
}

void QSimpleTimeline::setRangeLowFromCursorValue()
{
    setRangeLow(m_CursorValue);
}

void QSimpleTimeline::setRangeHighFromCursorValue()
{
    setRangeHigh(m_CursorValue);
}

unsigned int QSimpleTimeline::getStep() const
{
    return m_step;
}

void QSimpleTimeline::setStep(unsigned int const step)
{
    m_step = step;
}

void QSimpleTimeline::setRangeMinMax(unsigned int const minimum, unsigned int const maximum)
{
    setRangeMin(minimum);
    setRangeMax(maximum);
}

QSize QSimpleTimeline::sizeHint() const
{
    return QSize(100 * RANGE_HANDLE_WIDTH + 2 * SLIDER_PADDING,
                 2 * RANGE_HANDLE_HEIGHT + 2 * SLIDER_PADDING);
}

QSize QSimpleTimeline::minimumSizeHint() const
{
    return QSize(2 * RANGE_HANDLE_WIDTH + 2 * SLIDER_PADDING, 2 * RANGE_HANDLE_HEIGHT);
}

void QSimpleTimeline::mousePressEvent(QMouseEvent *e)
{
    // Check if event was on slider
    if (e->position().y() >= (height() - SLIDER_HEIGHT - RANGE_HANDLE_HEIGHT)
        && e->position().y() <= (height() - SLIDER_HEIGHT + RANGE_HANDLE_HEIGHT)) {
        float mouseX = e->position().x() < 0 ? 0 : e->position().x();
        m_lastMouseValue = (mouseX / width()) * (m_maximum - m_minimum) + m_minimum;

        bool bRequireUpdate = false;
        if (getIsHoveringElement(QTimelineElements::playerHandle, m_lastMouseValue)) {
            m_pressedElement = QTimelineElements::playerHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QTimelineElements::lowHandle, m_lastMouseValue)) {
            m_pressedElement = QTimelineElements::lowHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QTimelineElements::highHandle, m_lastMouseValue)) {
            m_pressedElement = QTimelineElements::highHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QTimelineElements::rangeHandle, m_lastMouseValue)) {
            m_pressedElement = QTimelineElements::rangeHandle;
            bRequireUpdate = true;
        }

        if (bRequireUpdate) {
            update();
        }
    }
}

void QSimpleTimeline::mouseReleaseEvent(QMouseEvent *e)
{
    m_lastMouseValue = -1;
    mouseMoveEvent(e);
}

void QSimpleTimeline::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    if (m_pressedElement == QTimelineElements::none) {
        m_pressedElement = QTimelineElements::none;
        m_hoveredElement = QTimelineElements::none;
        m_lastMouseValue = -1;
        update();
    }
}

void QSimpleTimeline::mouseMoveEvent(QMouseEvent *e)
{
    const float mouseX = e->position().x() < 0 ? 0 : e->position().x();
    const unsigned int mouseValue = (mouseX / width()) * (m_maximum - m_minimum) + m_minimum;
    const bool bIsClickDown = m_lastMouseValue != -1;
    bool bRequireUpdate = false;

    if (bIsClickDown) {
        switch (m_pressedElement) {
        case (QTimelineElements::none):
            break;
        case (QTimelineElements::playerHandle):
            setCursorValue(mouseValue);
            bRequireUpdate = true;
            break;
        case (QTimelineElements::lowHandle):
            setRangeLow(mouseValue);
            bRequireUpdate = true;
            break;
        case (QTimelineElements::highHandle):
            setRangeHigh(mouseValue);
            bRequireUpdate = true;
            break;
        case (QTimelineElements::rangeHandle):
            const int deltaValue = (mouseValue - m_lastMouseValue);
            if (deltaValue < 0) {
                setRangeLow(m_lowValue + deltaValue > m_lowValue
                                ? m_minimum
                                : m_lowValue + deltaValue); // Check fo underflow
                setRangeHigh(m_highValue + deltaValue);
            } else if (deltaValue > 0) {
                setRangeLow(m_lowValue + deltaValue);
                setRangeHigh(m_highValue + deltaValue < m_highValue
                                 ? m_maximum
                                 : m_highValue + deltaValue); // Check fo overflow
            }
            bRequireUpdate = true;
            break;
        }

        m_lastMouseValue = mouseValue;
    } else {
        if (m_hoveredElement != QTimelineElements::none) {
            bRequireUpdate = true;
        }
        // User not clicking any elements, check if he is hovering any
        m_pressedElement = QTimelineElements::none;
        m_hoveredElement = QTimelineElements::none;
        m_lastMouseValue = -1;

        if (getIsHoveringElement(QTimelineElements::playerHandle, mouseValue)) {
            m_hoveredElement = QTimelineElements::playerHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QTimelineElements::lowHandle, mouseValue)) {
            m_hoveredElement = QTimelineElements::lowHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QTimelineElements::highHandle, mouseValue)) {
            m_hoveredElement = QTimelineElements::highHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QTimelineElements::rangeHandle, mouseValue)) {
            m_hoveredElement = QTimelineElements::rangeHandle;
            bRequireUpdate = true;
        }
    }

    if (bRequireUpdate) {
        update();
    }
}

void QSimpleTimeline::paintEvent(QPaintEvent *event)
{
    updateScaleFactors();

    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);

    // Draw background
    painter.setPen(QPen(m_timelineBackgroundColor, 0.8));
    painter.setBrush(QBrush(m_timelineBackgroundColor));
    painter.drawRoundedRect(SLIDER_PADDING,
                            (height() - SLIDER_HEIGHT) / 2,
                            width() - 2 * SLIDER_PADDING,
                            SLIDER_HEIGHT,
                            2,
                            2);

    float const AbsRange = (m_maximum - m_minimum);
    float const AbsSelectedRange = (m_highValue - m_lowValue);
    float const AbsRangeValue = AbsSelectedRange / AbsRange;
    float const lowerHandleAbsValue = (m_lowValue - m_minimum) / AbsRange;
    float const higherHandleAbsValue = (m_highValue - m_minimum) / AbsRange;
    float const playerHandleAbsValue = (m_CursorValue) / AbsRange;

    // Draw range
    painter.setBrush(QBrush(m_hoveredElement == QTimelineElements::rangeHandle ? m_rangeColor_Hovered : m_rangeColor));
    painter.drawRect(SLIDER_PADDING + ((width() - 2 * SLIDER_PADDING) * lowerHandleAbsValue),
                     (height() - SLIDER_HEIGHT * m_rangeScaleFactor) / 2,
                     (width() - 2 * SLIDER_PADDING) * AbsRangeValue,
                     SLIDER_HEIGHT * m_rangeScaleFactor);

    // Draw lower handle
    painter.setBrush(QBrush(m_pressedElement == QTimelineElements::lowHandle ? m_rangeHandlesColor_Pressed
                                                                             : m_rangeHandlesColor_Normal));
    painter.drawRoundedRect(SLIDER_PADDING + ((width() - 2 * SLIDER_PADDING) * lowerHandleAbsValue)
                                - RANGE_HANDLE_WIDTH * lowerHandleAbsValue,
                            (height() - RANGE_HANDLE_HEIGHT
                             - (RANGE_HANDLE_HEIGHT * m_lowHandleScaleFactor - RANGE_HANDLE_HEIGHT))
                                / 2,
                            RANGE_HANDLE_WIDTH * m_lowHandleScaleFactor,
                            RANGE_HANDLE_HEIGHT * m_lowHandleScaleFactor,
                            2,
                            2);

    // Draw higher handle
    painter.setBrush(QBrush(m_pressedElement == QTimelineElements::highHandle ? m_rangeHandlesColor_Pressed
                                                                              : m_rangeHandlesColor_Normal));
    painter.drawRoundedRect(SLIDER_PADDING
                                + ((width() - 2 * SLIDER_PADDING) * higherHandleAbsValue
                                   - RANGE_HANDLE_WIDTH * higherHandleAbsValue),
                            (height() - RANGE_HANDLE_HEIGHT
                             - (RANGE_HANDLE_HEIGHT * m_highHandleScaleFactor - RANGE_HANDLE_HEIGHT))
                                / 2,
                            RANGE_HANDLE_WIDTH * m_highHandleScaleFactor,
                            RANGE_HANDLE_HEIGHT * m_highHandleScaleFactor,
                            2,
                            2);

    // Draw player handle
    painter.setBrush(QBrush(m_pressedElement == QTimelineElements::playerHandle ? m_rangeHandlesColor_Pressed
                                                                                : m_rangeHandlesColor_Normal));
    painter.drawRoundedRect(SLIDER_PADDING
                                + ((width() - 2 * SLIDER_PADDING) * playerHandleAbsValue
                                   - PLAYER_HANDLE_WIDTH * playerHandleAbsValue),
                            (height() - PLAYER_HANDLE_HEIGHT
                             - (PLAYER_HANDLE_HEIGHT * m_playerHandleScaleFactor
                                - PLAYER_HANDLE_HEIGHT))
                                / 2,
                            PLAYER_HANDLE_WIDTH * m_playerHandleScaleFactor,
                            PLAYER_HANDLE_HEIGHT * m_playerHandleScaleFactor,
                            2,
                            2);

    painter.end();
}

bool QSimpleTimeline::getIsHoveringElement(QTimelineElements const element, int const mouseValue) const
{
    // NOTE: Casting to int since we are treating the values as coordinates rather than
    // milieseconds. We need to make sure values < 0 aren't a problem.

    bool bIsHoveringElement = false;

    /* Effective size of handles (changes when the timeline is scaled up/down) */
    int handleSizeValue = (RANGE_HANDLE_WIDTH * (m_maximum - m_minimum)) / width();
    int playerHandleSizeValue = (PLAYER_HANDLE_WIDTH * (m_maximum - m_minimum)) / width();

    switch (element) {
    case (QTimelineElements::none):
        break;
    case (QTimelineElements::playerHandle):
        bIsHoveringElement = mouseValue >= int(m_CursorValue) - int(playerHandleSizeValue)
                             && mouseValue <= int(m_CursorValue) + int(playerHandleSizeValue);
        break;
    case (QTimelineElements::lowHandle):
        bIsHoveringElement = mouseValue >= int(m_lowValue) - int(handleSizeValue)
                             && mouseValue <= int(m_lowValue) + int(handleSizeValue);
        break;
    case (QTimelineElements::highHandle):
        bIsHoveringElement = mouseValue >= int(m_highValue) - int(handleSizeValue)
                             && mouseValue <= int(m_highValue) + int(handleSizeValue);
        break;
    case (QTimelineElements::rangeHandle):
        bIsHoveringElement = mouseValue < m_highValue && mouseValue > m_lowValue;
        break;
    }

    return bIsHoveringElement;
}

void QSimpleTimeline::updateScaleFactors()
{
    m_lowHandleScaleFactor = 1.0;
    m_highHandleScaleFactor = 1.0;
    m_rangeScaleFactor = 1.0;
    m_playerHandleScaleFactor = 1.0;

    switch (m_hoveredElement) {
    case (QTimelineElements::none):
        break;
    case (QTimelineElements::playerHandle):
        m_playerHandleScaleFactor = PLAYER_HANDLE_HOVERED_SCALE;
        break;
    case (QTimelineElements::lowHandle):
        m_lowHandleScaleFactor = RANGE_HANDLES_HOVERED_SCALE;
        break;
    case (QTimelineElements::highHandle):
        m_highHandleScaleFactor = RANGE_HANDLES_HOVERED_SCALE;
        break;
    case (QTimelineElements::rangeHandle):
        m_rangeScaleFactor = RANGE_HOVERED_SCALE;
        break;
    }
}

void QSimpleTimeline::updateColorsFromStylesheet()
{
    QStyleManager &styleManager = QStyleManager::getInstance();

    QStyleManager::QAppStyle ActiveStyle = styleManager.getActiveStyle();
    //QStyleManager::QAppStyle::Dark;
    QString ActiveStyleFilename = styleManager.getStyleFileName(ActiveStyle);
    m_rangeHandlesColor_Normal = styleManager.parseQssFileForColour(ActiveStyleFilename,
                                                                    "QSlider::handle:horizontal",
                                                                    "background");
    m_rangeHandlesColor_Pressed = styleManager.parseQssFileForColour(ActiveStyleFilename,
                                                                     "QSlider::handle:horizontal:hover",
                                                                     "background");
    m_playHandlesColor_Normal = styleManager.parseQssFileForColour(ActiveStyleFilename,
                                                                   "QPushButton:pressed",
                                                                   "background-color");
    m_playHandlesColor_Pressed = styleManager.parseQssFileForColour(ActiveStyleFilename,
                                                                    "QSlider::handle:horizontal",
                                                                    "background");
    m_timelineBackgroundColor = styleManager.parseQssFileForColour(ActiveStyleFilename,
                                                                   "QSlider::groove:horizontal",
                                                                   "background");
    m_rangeColor = styleManager.parseQssFileForColour(ActiveStyleFilename, "QSlider::handle:horizontal:focus", "border");

    m_rangeColor_Hovered = styleManager.parseQssFileForColour(ActiveStyleFilename,
                                                              "QSlider::handle:horizontal:hover",
                                                              "background");
}
