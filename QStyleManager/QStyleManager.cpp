#include "QStyleManager.h"
#include "QTextStreamManipulator"
#include "qapplication.h"
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>
#include <regex>
#include <string>

QStyleManager::QStyleManager() {}

void QStyleManager::setStyle(QApplication* App, const QAppStyle Style)
{
    if (App) {
        QString StylePath = getStyleFileName(Style);

        QFile f(StylePath);
        if (!f.exists()) {
            printf("Unable to set stylesheet, file not found\n");
        } else {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            App->setStyleSheet(ts.readAll());
            m_activeStyle = Style;
        }
    }
}

QStyleManager::QAppStyle QStyleManager::getActiveStyle()
{
    return m_activeStyle;
}

QList<QString> QStyleManager::parseQssFileForString(const QString &qssFilePath,
                                                    const QString &targetBlock,
                                                    const QString &targetProperty)
{
    QList<QString> retrievedStrings;

    QFile qssFile(qssFilePath);
    if (qssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream qssStream(&qssFile);
        QString qssContent = qssStream.readAll();
        qssFile.close();

        QString targetSelector = targetBlock + " {";
        int targetIndex = qssContent.indexOf(targetSelector);
        if (targetIndex != -1) {
            int startIndex = targetIndex + targetSelector.length();
            int endIndex = qssContent.indexOf('}', startIndex);
            QString blockContent = qssContent.mid(startIndex, endIndex - startIndex);

            QRegularExpression propertyRegex(targetProperty + "\\s*:\\s*([^;]+)");
            QRegularExpressionMatchIterator matchIterator = propertyRegex.globalMatch(blockContent);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                QString propertyString = match.captured(1);
                retrievedStrings.append(propertyString);
            }
        }
    }

    return retrievedStrings;
}

QColor QStyleManager::parseQssFileForColour(const QString &qssFilePath,
                                            const QString &targetBlock,
                                            const QString &targetProperty)
{
    QList<QString> retrievedProperties = parseQssFileForString(qssFilePath,
                                                               targetBlock,
                                                               targetProperty);
    QColor returnValue = Qt::GlobalColor::black;

    for (int i = 0; i < retrievedProperties.length(); i++) {
        std::string regexString = retrievedProperties[i].toStdString();
        std::regex colorRegex("#([A-Fa-f0-9]{6})"); // Regular expression pattern to match the color
        std::smatch regexMatches;

        if (std::regex_search(regexString, regexMatches, colorRegex)) {
            QString color = QString::fromStdString(regexMatches[1].str());
            returnValue = QColor("#" + color);
            if (returnValue.isValid()) {
                return returnValue;
            }
        }
    }

    qDebug() << "parseQssFileForColour: Color property could not be retrieved.";
    return returnValue;
}

QString QStyleManager::getStyleFileName(QAppStyle Style)
{
    QString StylePath = "";
    switch (Style) {
    case QAppStyle::Dark:
        StylePath = DARKSTYLEPATH;
        break;

    case QAppStyle::Light:
        StylePath = LIGHTSTYLEPATH;
        break;

    default:
        StylePath = DARKSTYLEPATH;
        break;
    }

    return StylePath;
}
