#ifndef QVIDEOPLAYERWIDGET_H
#define QVIDEOPLAYERWIDGET_H

#include <QGraphicsView>
#include <QVector2D>
#include <QWidget>

class QGraphicsVideoItem;

enum QCropElements { cropnone, topleftHandle, toprightHandle, bottomleftHandle, bottomrightHandle, wholeHandle };

class QVideoPlayerWidget : public QGraphicsView
{
public:
    QVideoPlayerWidget();

    /* Reset crop and video rectangle to match new media file resolution. */
    void initialiseFromNewMedia();

    /* Returns the video output to use in yyour QMediaPlayer. */
    QGraphicsVideoItem *getVideoOutput() const;

    /* Returns the top left position of the crop area in percent (0-1). */
    QPointF getCropPosPercent() const;

    /* Returns the size of the crop area in percent (0-1). */
    QSizeF getCropSizePercent() const;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void resizeVideoPlayer(QSizeF const newSize);

    /* Evaluate if MousePos is overlapping with an CropElement. */
    bool getIsHoveringElement(QCropElements const element, QPoint const MousePos);

private:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    QGraphicsScene *m_graphicsScene = nullptr;
    QGraphicsVideoItem *m_graphicsVideoItem = nullptr;

    /* Triggered when the first frame of a new media file is recieved, sets up crop and video rect. */
    void onFirstFrameRecieved();

    /* Set the positions of any of the QCropElements. */
    void setCropHandlePos(QCropElements element, QPointF MousePos);

    /* QRect representing the video's rendering area on screen. Serves as bounds for the crop area. */
    QRectF m_videoRectangle = QRectF();
    /* QRect representing the user's crop selection. */
    QRectF m_cropRectangle = QRectF();

    /* When pressed, represents the position of the mouse on screen. */
    QPoint m_lastMouseValue = QPoint(-1, -1);

    /* Currently hovered element. */
    QCropElements m_hoveredElement = QCropElements::cropnone;
    /* Currently pressed element.  */
    QCropElements m_pressedElement = QCropElements::cropnone;

    /* Scales elements based on hover events */
    void updateScaleFactors();
    float m_topleftHandleScale = 1.F;
    float m_toprightHandleScale = 1.F;
    float m_bottomleftHandleScale = 1.F;
    float m_bottomrightHandleScale = 1.F;

    /* Colors dynamically read from active QSS file. */
    void updateColorsFromStylesheet();
    QColor m_cropHandlesColor_Normal = QColor(Qt::GlobalColor::white);
    QColor m_cropHandlesColor_Pressed = QColor(Qt::GlobalColor::darkGray);
    QColor m_cropLinesColor = QColor(Qt::GlobalColor::darkGray);
    QColor m_cropAreaColor_Hovered = QColor(64, 64, 64, 1);

    /* Painter constants */
    /* Size of the corner handles for the crop area (square). */
    const unsigned int CROP_HANDLE_SIZE = 10;
    /* Thickness of the lines between the corner handles. */
    const unsigned int CROP_LINES_THICKNESS = 2;
    /* Minimum size of the crop area (prevents handle overlapping). */
    const unsigned int CROP_AREA_MINIMUM_SIZE = 20;
    /* Crop handle scale multiplier when hovered. */
    static constexpr float CROP_HANDLES_HOVERED_SCALE = 1.3;
};

#endif // QVIDEOPLAYERWIDGET_H
