#ifndef QSTYLEMANAGER_H
#define QSTYLEMANAGER_H

#include <QObject>

class QApplication;

class QStyleManager : public QObject
{
    Q_OBJECT
private:
    QStyleManager();                                 // Private constructor
    QStyleManager(const QStyleManager &);            // Private copy constructor
    QStyleManager &operator=(const QStyleManager &); // Private assignment operator

    // Style definitions
private:
    const QString DARKSTYLEPATH = ":QStyleManager/dark/darkstyle.qss";
    const QString LIGHTSTYLEPATH = ":QStyleManager/light/lightstyle.qss";

public:
    enum QAppStyle { Dark, Light };

public:
    static QStyleManager &getInstance()
    {
        static QStyleManager instance;
        return instance;
    }

    void setStyle(QApplication *App, const QAppStyle Style);
    QAppStyle getActiveStyle();
    QString getStyleFileName(QAppStyle Style);
    QList<QString> parseQssFileForString(const QString &qssFilePath,
                                         const QString &targetBlock,
                                         const QString &targetProperty);
    QColor parseQssFileForColour(const QString &qssFilePath,
                                 const QString &targetBlock,
                                 const QString &targetProperty);

private:
    QAppStyle m_activeStyle;

signals:

};

#endif // QSTYLEMANAGER_H
