#include "mainwindow.h"
#include "tracktypes.h"

#include <QApplication>

#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <QMutex>
#include <QDateTime>

void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString strLogRoot = QApplication::applicationDirPath() + QDir::separator() + "log";
    strLogRoot = QDir::toNativeSeparators(strLogRoot);
    QDir dir(strLogRoot);
    if (!dir.exists())
    {
        dir.mkpath(strLogRoot);
    }

    static QMutex mutex;
    mutex.lock();

    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString current_date = QString("(%1)").arg(current_date_time);
    QString message = QString("%1 %2").arg(current_date).arg(msg);

    switch(type)
    {
    case QtDebugMsg:
    {
        QString strFile = strLogRoot + QDir::separator() + QString("%1_DebugLog.txt").arg(QDate::currentDate().toString("yyyy-MM-dd"));
        QFile file(strFile);
        file.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream text_stream(&file);
        text_stream << message << "\r\n";
        file.flush();
        file.close();
    }
        break;

    case QtWarningMsg:
    {
        QString strFile2 = strLogRoot + QDir::separator() + QString("%1_ErrorWarningLog.txt").arg(QDate::currentDate().toString("yyyy-MM-dd"));
        QFile file2(strFile2);
        file2.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream text_stream2(&file2);
        text_stream2 << message << "\r\n";
        file2.flush();
        file2.close();
    }
        break;

    case QtCriticalMsg:
    {
        QString strFile3 = strLogRoot + QDir::separator() + QString("%1_ErrorWarningLog.txt").arg(QDate::currentDate().toString("yyyy-MM-dd"));
        QFile file3(strFile3);
        file3.open(QIODevice::WriteOnly | QIODevice::Append);
        QTextStream text_stream3(&file3);
        text_stream3 << message << "\r\n";
        file3.flush();
        file3.close();
    }
        break;

    case QtFatalMsg:
        break;
    }

    mutex.unlock();
}

void registerMetaClass()
{
	qRegisterMetaType<Object>("Object");
	qRegisterMetaType<Object *>("Object*");
	qRegisterMetaType<Rect<double> >("Rect<double>");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //注册MessageHandler
	registerMetaClass();
#ifdef QT_NO_DEBUG
    qInstallMessageHandler(outputMessage);
    qDebug("release version");
#else
    qDebug("debug version");
#endif

    MainWindow w;
    w.show();

    return a.exec();
}
