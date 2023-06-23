#include "QVideoPlayerWidget.h"
#include "QStyleManager.h"
#include "qevent.h"
#include "qpainter.h"
#include <QGraphicsVideoItem>
#include <QGraphicsView>
#include <QSizePolicy>

QVideoPlayerWidget::QVideoPlayerWidget()
{
    // View setup
    setAcceptDrops(false);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setViewportUpdateMode(QGraphicsView::ViewportUpdateMode::FullViewportUpdate);
    setTransformationAnchor(ViewportAnchor::AnchorViewCenter);

    // Video item & scene setup
    m_graphicsVideoItem = new QGraphicsVideoItem();
    m_graphicsVideoItem->setAspectRatioMode(Qt::KeepAspectRatio);
    m_graphicsScene = new QGraphicsScene(this);
    m_graphicsScene->addItem(m_graphicsVideoItem);

    setScene(m_graphicsScene);
    updateColorsFromStylesheet();
}

void QVideoPlayerWidget::initialiseFromNewMedia()
{
    connect(m_graphicsVideoItem,
            &QGraphicsVideoItem::nativeSizeChanged,
            this,
            &QVideoPlayerWidget::onFirstFrameRecieved);
}

void QVideoPlayerWidget::onFirstFrameRecieved()
{
    // Requires the media to be  buffered and displayed in VideoSink
    qDebug() << "QVideoPlayerWidget: Initialised with new media.";

    m_videoRectangle = QRectF();
    m_cropRectangle = QRectF();
    resizeVideoPlayer(viewport()->size());

    disconnect(m_graphicsVideoItem,
               &QGraphicsVideoItem::nativeSizeChanged,
               this,
               &QVideoPlayerWidget::onFirstFrameRecieved);
}

void QVideoPlayerWidget::setCropHandlePos(QCropElements element, QPointF MousePos)
{
    QPointF newPos;
    switch (element) {
    case (QCropElements::cropnone):
        break;
    case (QCropElements::topleftHandle):
        newPos = QPointF(std::clamp(MousePos.x(),
                                    m_videoRectangle.topLeft().x(),
                                    m_cropRectangle.bottomRight().x() - CROP_AREA_MINIMUM_SIZE),
                         std::clamp(MousePos.y(),
                                    m_videoRectangle.topLeft().y(),
                                    m_cropRectangle.bottomRight().y() - CROP_AREA_MINIMUM_SIZE));
        m_cropRectangle.setTopLeft(newPos);
        break;
    case (QCropElements::toprightHandle):
        newPos = QPointF(std::clamp(MousePos.x(),
                                    m_cropRectangle.topLeft().x() + CROP_AREA_MINIMUM_SIZE,
                                    m_videoRectangle.bottomRight().x()),
                         std::clamp(MousePos.y(),
                                    m_videoRectangle.topLeft().y(),
                                    m_cropRectangle.bottomRight().y() - CROP_AREA_MINIMUM_SIZE));
        m_cropRectangle.setTopRight(newPos);
        break;
    case (QCropElements::bottomleftHandle):
        newPos = QPointF(std::clamp(MousePos.x(),
                                    m_videoRectangle.topLeft().x(),
                                    m_cropRectangle.bottomRight().x() - CROP_AREA_MINIMUM_SIZE),
                         std::clamp(MousePos.y(),
                                    m_cropRectangle.topLeft().y() + CROP_AREA_MINIMUM_SIZE,
                                    m_videoRectangle.bottomRight().y()));
        m_cropRectangle.setBottomLeft(newPos);
        break;
    case (QCropElements::bottomrightHandle):
        newPos = QPointF(std::clamp(MousePos.x(),
                                    m_cropRectangle.topLeft().x() + CROP_AREA_MINIMUM_SIZE,
                                    m_videoRectangle.bottomRight().x()),
                         std::clamp(MousePos.y(),
                                    m_cropRectangle.topLeft().y() + CROP_AREA_MINIMUM_SIZE,
                                    m_videoRectangle.bottomRight().y()));
        m_cropRectangle.setBottomRight(newPos);
        break;
    case (QCropElements::wholeHandle):

        QPointF deltaMouse = MousePos - m_lastMouseValue;

        QPointF newTopLeft = m_cropRectangle.topLeft() + deltaMouse;
        QPointF newBottomRight = m_cropRectangle.bottomRight() + deltaMouse;
        newTopLeft = QPointF(std::clamp(newTopLeft.x(),
                                        m_videoRectangle.topLeft().x(),
                                        m_cropRectangle.bottomRight().x() - CROP_AREA_MINIMUM_SIZE),
                             std::clamp(newTopLeft.y(),
                                        m_videoRectangle.topLeft().y(),
                                        m_cropRectangle.bottomRight().y() - CROP_AREA_MINIMUM_SIZE));

        newBottomRight = QPointF(std::clamp(newBottomRight.x(),
                                            m_cropRectangle.topLeft().x() + CROP_AREA_MINIMUM_SIZE,
                                            m_videoRectangle.bottomRight().x()),
                                 std::clamp(newBottomRight.y(),
                                            m_cropRectangle.topLeft().y() + CROP_AREA_MINIMUM_SIZE,
                                            m_videoRectangle.bottomRight().y()));

        m_cropRectangle.setCoords(newTopLeft.x(), newTopLeft.y(), newBottomRight.x(), newBottomRight.y());

        break;
    }
}

QGraphicsVideoItem *QVideoPlayerWidget::getVideoOutput() const
{
    return m_graphicsVideoItem;
}

QPointF QVideoPlayerWidget::getCropPosPercent() const
{
    const QPointF cropPosAdjusted = QPointF(fmax(m_cropRectangle.topLeft().x() - m_videoRectangle.x(), 0.F),
                                            fmax(m_cropRectangle.topLeft().y() - m_videoRectangle.y(), 0.F));

    const QPointF cropPosPercent = QPointF(cropPosAdjusted.x() / m_videoRectangle.width(),
                                           cropPosAdjusted.y() / m_videoRectangle.height());
    return cropPosPercent;
}

QSizeF QVideoPlayerWidget::getCropSizePercent() const
{
    const QSizeF cropSizePercent = QSizeF(m_cropRectangle.width() / m_videoRectangle.width(),
                                          m_cropRectangle.height() / m_videoRectangle.height());
    return cropSizePercent;
}

void QVideoPlayerWidget::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    resizeVideoPlayer(event->size());
}

void QVideoPlayerWidget::resizeVideoPlayer(QSizeF const newSize)
{
    if (m_graphicsVideoItem != NULL) {
        // Resize and reposition VideoItem
        m_graphicsVideoItem->setSize(newSize);
        m_graphicsScene->setSceneRect(m_graphicsVideoItem->boundingRect());

        // Update video rectangle & scale proportionnally current crop rectangle
        const QSizeF cropSizePercent = getCropSizePercent();
        const QPointF cropPosPercent = getCropPosPercent();

        QRectF oldRectangle = m_videoRectangle;
        m_videoRectangle = m_graphicsVideoItem->boundingRect();

        if (m_cropRectangle != QRectF()) {
            // Scale and move crop area to fit the new window size
            const QPointF newTopLeft = QPointF(cropPosPercent.x() * m_videoRectangle.width() + m_videoRectangle.x(),
                                               cropPosPercent.y() * m_videoRectangle.height() + m_videoRectangle.y());
            const QPointF newBottomRight = QPointF(newTopLeft.x() + (cropSizePercent.width() * m_videoRectangle.width()),
                                                   newTopLeft.y()
                                                       + (cropSizePercent.height() * m_videoRectangle.height()));

            m_cropRectangle.setCoords(newTopLeft.x(), newTopLeft.y(), newBottomRight.x(), newBottomRight.y());

        } else {
            // cropRectangle is null or unchanged, ensure it stays the same as max size
            m_cropRectangle = m_videoRectangle;
        }
    } else {
        qDebug() << "resizeVideoPlayer: Failed to resize, m_graphicsVideoItem is null.";
    }
}

bool QVideoPlayerWidget::getIsHoveringElement(QCropElements const element, QPoint const MousePos)
{
    bool bIsHoveringElem = false;

    switch (element) {
    case (QCropElements::cropnone):
        break;
    case (QCropElements::topleftHandle):
        bIsHoveringElem = QLineF(m_cropRectangle.topLeft(), MousePos).length() <= (CROP_HANDLE_SIZE / 2);
        break;
    case (QCropElements::toprightHandle):
        bIsHoveringElem = QLineF(m_cropRectangle.topRight(), MousePos).length() <= (CROP_HANDLE_SIZE / 2);
        break;
    case (QCropElements::bottomleftHandle):
        bIsHoveringElem = QLineF(m_cropRectangle.bottomLeft(), MousePos).length() <= (CROP_HANDLE_SIZE / 2);
        break;
    case (QCropElements::bottomrightHandle):
        bIsHoveringElem = QLineF(m_cropRectangle.bottomRight(), MousePos).length() <= (CROP_HANDLE_SIZE / 2);
        break;
    case (QCropElements::wholeHandle):
        bIsHoveringElem = m_cropRectangle.contains(MousePos);
        break;
    }

    return bIsHoveringElem;
}

void QVideoPlayerWidget::mousePressEvent(QMouseEvent *e)
{
    m_lastMouseValue = QPoint(-1, -1);
    const QPoint mouseValue = QPoint(e->position().x(), e->position().y());

    const float mouseXValue = fmin(fmax(mouseValue.x(), m_videoRectangle.left()), m_videoRectangle.right());
    const float mouseYValue = fmin(fmax(mouseValue.y(), m_videoRectangle.top()), m_videoRectangle.bottom());

    m_lastMouseValue = QPoint(mouseXValue, mouseYValue);

    bool bRequireUpdate = false;
    if (m_hoveredElement == QCropElements::cropnone) {
        if (getIsHoveringElement(QCropElements::topleftHandle, mouseValue)) {
            m_pressedElement = QCropElements::topleftHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QCropElements::toprightHandle, mouseValue)) {
            m_pressedElement = QCropElements::toprightHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QCropElements::bottomleftHandle, mouseValue)) {
            m_pressedElement = QCropElements::bottomleftHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QCropElements::bottomrightHandle, mouseValue)) {
            m_pressedElement = QCropElements::bottomrightHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QCropElements::wholeHandle, mouseValue)) {
            m_pressedElement = QCropElements::wholeHandle;
            bRequireUpdate = true;
        }

    } else {
        m_pressedElement = m_hoveredElement;
        bRequireUpdate = true;
    }

    if (bRequireUpdate) {
        viewport()->update();
    }
}

void QVideoPlayerWidget::mouseReleaseEvent(QMouseEvent *e)
{
    m_lastMouseValue = QPoint(-1, -1);
    mouseMoveEvent(e);
}

void QVideoPlayerWidget::mouseMoveEvent(QMouseEvent *e)
{
    const QPoint mouseValue = QPoint(e->position().x(), e->position().y());
    const bool bIsClickDown = m_lastMouseValue != QPoint(-1, -1);
    bool bRequireUpdate = false;

    if (bIsClickDown) {
        switch (m_pressedElement) {
        case (QCropElements::cropnone):
            break;
        case (QCropElements::topleftHandle):
            bRequireUpdate = true;
            break;
        case (QCropElements::toprightHandle):
            bRequireUpdate = true;
            break;
        case (QCropElements::bottomleftHandle):
            bRequireUpdate = true;
            break;
        case (QCropElements::bottomrightHandle):
            bRequireUpdate = true;
            break;
        case (QCropElements::wholeHandle):
            bRequireUpdate = true;
            break;
        }
        setCropHandlePos(m_pressedElement, mouseValue);
        m_lastMouseValue = mouseValue;

    } else {
        if (m_hoveredElement != QCropElements::cropnone) {
            bRequireUpdate = true;
        }
        m_pressedElement = QCropElements::cropnone;
        m_hoveredElement = QCropElements::cropnone;
        m_lastMouseValue = QPoint(-1, -1);

        if (getIsHoveringElement(QCropElements::topleftHandle, mouseValue)) {
            m_hoveredElement = QCropElements::topleftHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QCropElements::toprightHandle, mouseValue)) {
            m_hoveredElement = QCropElements::toprightHandle;
        } else if (getIsHoveringElement(QCropElements::bottomleftHandle, mouseValue)) {
            m_hoveredElement = QCropElements::bottomleftHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QCropElements::bottomrightHandle, mouseValue)) {
            m_hoveredElement = QCropElements::bottomrightHandle;
            bRequireUpdate = true;
        } else if (getIsHoveringElement(QCropElements::wholeHandle, mouseValue)) {
            m_hoveredElement = QCropElements::wholeHandle;
            bRequireUpdate = true;
        }
    }

    if (bRequireUpdate) {
        viewport()->update();
    }
}

void QVideoPlayerWidget::leaveEvent(QEvent *event)
{
    if (m_pressedElement == QCropElements::cropnone) {
        m_pressedElement = QCropElements::cropnone;
        m_hoveredElement = QCropElements::cropnone;
        m_lastMouseValue = QPoint(-1, -1);
        viewport()->update();
    }
}

void QVideoPlayerWidget::paintEvent(QPaintEvent *event)
{
    updateScaleFactors();
    QGraphicsView::paintEvent(event);
    QPainter painter(this->viewport());
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);

    // Draw lines between handles
    painter.setPen(QPen(m_cropLinesColor, CROP_LINES_THICKNESS));
    QList<QLineF> LinesToDraw;
    LinesToDraw.append(QLineF(m_cropRectangle.topLeft(), m_cropRectangle.topRight()));
    LinesToDraw.append(QLineF(m_cropRectangle.topRight(), m_cropRectangle.bottomRight()));
    LinesToDraw.append(QLineF(m_cropRectangle.bottomRight(), m_cropRectangle.bottomLeft()));
    LinesToDraw.append(QLineF(m_cropRectangle.bottomLeft(), m_cropRectangle.topLeft()));
    painter.drawLines(LinesToDraw);

    // Draw crop area when hovered
    if (m_hoveredElement == QCropElements::wholeHandle) {
        painter.setBrush(QBrush(m_cropAreaColor_Hovered));
        painter.drawRect(m_cropRectangle);
    }

    // Draw top left handle
    painter.setBrush(QBrush(m_pressedElement == QCropElements::topleftHandle ? m_cropHandlesColor_Pressed
                                                                             : m_cropHandlesColor_Normal));
    painter.drawRoundedRect(m_cropRectangle.topLeft().x() - (CROP_HANDLE_SIZE * m_topleftHandleScale / 2),
                            m_cropRectangle.topLeft().y() - (CROP_HANDLE_SIZE * m_topleftHandleScale / 2),
                            CROP_HANDLE_SIZE * m_topleftHandleScale,
                            CROP_HANDLE_SIZE * m_topleftHandleScale,
                            2,
                            2);

    // Draw top right handle
    painter.setBrush(QBrush(m_pressedElement == QCropElements::toprightHandle ? m_cropHandlesColor_Pressed
                                                                              : m_cropHandlesColor_Normal));
    painter.drawRoundedRect(m_cropRectangle.topRight().x() - (CROP_HANDLE_SIZE * m_toprightHandleScale / 2),
                            m_cropRectangle.topRight().y() - (CROP_HANDLE_SIZE * m_toprightHandleScale / 2),
                            CROP_HANDLE_SIZE * m_toprightHandleScale,
                            CROP_HANDLE_SIZE * m_toprightHandleScale,
                            2,
                            2);

    // Draw bottom left handle
    painter.setBrush(QBrush(m_pressedElement == QCropElements::bottomleftHandle ? m_cropHandlesColor_Pressed
                                                                                : m_cropHandlesColor_Normal));
    painter.drawRoundedRect(m_cropRectangle.bottomLeft().x() - (CROP_HANDLE_SIZE * m_bottomleftHandleScale / 2),
                            m_cropRectangle.bottomLeft().y() - (CROP_HANDLE_SIZE * m_bottomleftHandleScale / 2),
                            CROP_HANDLE_SIZE * m_bottomleftHandleScale,
                            CROP_HANDLE_SIZE * m_bottomleftHandleScale,
                            2,
                            2);

    // Draw bottom right handle
    painter.setBrush(QBrush(m_pressedElement == QCropElements::bottomrightHandle ? m_cropHandlesColor_Pressed
                                                                                 : m_cropHandlesColor_Normal));
    painter.drawRoundedRect(m_cropRectangle.bottomRight().x() - (CROP_HANDLE_SIZE * m_bottomrightHandleScale / 2),
                            m_cropRectangle.bottomRight().y() - (CROP_HANDLE_SIZE * m_bottomrightHandleScale / 2),
                            CROP_HANDLE_SIZE * m_bottomrightHandleScale,
                            CROP_HANDLE_SIZE * m_bottomrightHandleScale,
                            2,
                            2);

    painter.end();
}

void QVideoPlayerWidget::updateScaleFactors()
{
    m_topleftHandleScale = 1.F;
    m_toprightHandleScale = 1.F;
    m_bottomleftHandleScale = 1.F;
    m_bottomrightHandleScale = 1.F;

    switch (m_hoveredElement) {
    case (QCropElements::cropnone):
        break;
    case (QCropElements::topleftHandle):
        m_topleftHandleScale = CROP_HANDLES_HOVERED_SCALE;
        break;
    case (QCropElements::toprightHandle):
        m_toprightHandleScale = CROP_HANDLES_HOVERED_SCALE;
        break;
    case (QCropElements::bottomleftHandle):
        m_bottomleftHandleScale = CROP_HANDLES_HOVERED_SCALE;
        break;
    case (QCropElements::bottomrightHandle):
        m_bottomrightHandleScale = CROP_HANDLES_HOVERED_SCALE;
        break;
    case (QCropElements::wholeHandle):
        break;
    }
}

void QVideoPlayerWidget::updateColorsFromStylesheet()
{
    QStyleManager &styleManager = QStyleManager::getInstance();

    QStyleManager::QAppStyle ActiveStyle = styleManager.getActiveStyle();
    QString ActiveStyleFilename = styleManager.getStyleFileName(ActiveStyle);

    m_cropHandlesColor_Normal = styleManager.parseQssFileForColour(ActiveStyleFilename,
                                                                   "QPushButton:pressed",
                                                                   "background-color");
    m_cropHandlesColor_Pressed = styleManager.parseQssFileForColour(ActiveStyleFilename,
                                                                    "QPushButton",
                                                                    "background-color");

    m_cropLinesColor = styleManager.parseQssFileForColour(ActiveStyleFilename, "QMenu::item:selected", "color");
    m_cropLinesColor.setAlphaF(0.5F);

    m_cropAreaColor_Hovered = styleManager.parseQssFileForColour(ActiveStyleFilename, "QMenu::item:selected", "color");
    m_cropAreaColor_Hovered.setAlphaF(0.1F);
}
