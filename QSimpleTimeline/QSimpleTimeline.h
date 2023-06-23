#include <QWidget>

enum QTimelineElements { none, playerHandle, lowHandle, highHandle, rangeHandle };

class QSimpleTimeline : public QWidget
{
    Q_OBJECT

public:
    QSimpleTimeline(QWidget *parent = nullptr);
    ~QSimpleTimeline();

    unsigned int getRangeMin() const;
    unsigned int getRangeMax() const;
    unsigned int getRangeLow() const;
    unsigned int getRangeHigh() const;
    unsigned int getCursorValue() const;

    unsigned int getStep() const;
    void setStep(unsigned int const step);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void setRangeMin(unsigned int const minimum);
    void setRangeMax(unsigned int const maximum);
    void setRangeLow(unsigned int const lowValue);
    void setRangeHigh(unsigned int const highValue);
    void setRangeMinMax(unsigned int const minimum, unsigned int const maximum);
    void setCursorValue(unsigned int const Value);
    void setRangeLowFromCursorValue();
    void setRangeHighFromCursorValue();

signals:
    void minimumChanged(unsigned int const minimum);
    void maximumChanged(unsigned int const maximum);
    void lowValueChanged(unsigned int const lowValue);
    void highValueChanged(unsigned int const highValue);
    void rangeChanged(unsigned int const lowValue, unsigned int const highValue);
    void cursorValueChanged(unsigned int const Value);

private:
    void mousePressEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

    bool getIsHoveringElement(QTimelineElements const element, int const mouseValue) const;

    /* Minimum range value */
    unsigned int m_minimum = 0;

    /* Maximum range value */
    unsigned int m_maximum = 1000;

    /* Step value */
    unsigned int m_step = 1;

    /* Value of low handle */
    unsigned int m_lowValue;

    /* Value of high handle */
    unsigned int m_highValue;

    /* Value of the cursor handle (playhead) */
    unsigned int m_CursorValue = 0.F;

    /* Value of the last mouse click */
    int m_lastMouseValue = -1;

    /* Currently hovered element */
    QTimelineElements m_hoveredElement = QTimelineElements::none;

    /* Currently pressed element */
    QTimelineElements m_pressedElement = QTimelineElements::none;

    /* Colors dynamically read from active QSS file. */
    void updateColorsFromStylesheet();
    QColor m_rangeHandlesColor_Normal = QColor(Qt::GlobalColor::white);
    QColor m_rangeHandlesColor_Pressed = QColor(Qt::GlobalColor::darkGray);
    QColor m_playHandlesColor_Normal = QColor(Qt::GlobalColor::white);
    QColor m_playHandlesColor_Pressed = QColor(Qt::GlobalColor::darkGray);
    QColor m_timelineBackgroundColor = QColor(0x1E, 0x90, 0xFF);
    QColor m_rangeColor = QColor(Qt::GlobalColor::darkGray);
    QColor m_rangeColor_Hovered = QColor(Qt::GlobalColor::lightGray);

    /* Scales elements based on hover events */
    void updateScaleFactors();
    float m_lowHandleScaleFactor = 1.F;
    float m_highHandleScaleFactor = 1.F;
    float m_rangeScaleFactor = 1.F;
    float m_playerHandleScaleFactor = 1.F;

    /* Painter constants */
    /* The minimum range size in value units (usually ms) */
    static constexpr unsigned int MINIMUM_RANGE = 40;
    /* Whole timeline height in pixels */
    static constexpr unsigned int SLIDER_HEIGHT = 40;
    /* Left and Right timeline padding in pixels */
    static constexpr unsigned int SLIDER_PADDING = 0;

    /* Range handles (low and high) height in pixels */
    static constexpr unsigned int RANGE_HANDLE_HEIGHT = 40;
    /* Range handles (low and high) width in pixels */
    static constexpr unsigned int RANGE_HANDLE_WIDTH = 10;
    /* Player handle height in pixels */
    static constexpr unsigned int PLAYER_HANDLE_HEIGHT = 60;
    /* Player handle width in pixels */
    static constexpr unsigned int PLAYER_HANDLE_WIDTH = 5;

    /* Range handles (low and high) hovered scale multiplier */
    static constexpr float RANGE_HANDLES_HOVERED_SCALE = 1.2;
    /* Range handles (low and high) hovered scale multiplier */
    static constexpr float RANGE_HOVERED_SCALE = 1.0;
    /* Player handle  hovered scale multiplier*/
    static constexpr float PLAYER_HANDLE_HOVERED_SCALE = 1.2;
};
