

#include <QApplication>
#include <QScreen>
#include <QStyleHints>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

#include "dialog.h"
#include "../voltio.hpp"



int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);

    qRegisterMetaType<interThreadData>();

    QGuiApplication::setApplicationDisplayName(Dialog::tr("Voltmeter Test Example"));

#ifndef QT_NO_TRANSLATION
    QString translatorFileName = QLatin1String("qtbase_");
    translatorFileName += QLocale::system().name();
    QTranslator *translator = new QTranslator(&app);
    if (translator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(translator);
#endif
    
    std::string path = voltio::default_socket_path;

    Dialog dialog(path);
    if (!QGuiApplication::styleHints()->showIsFullScreen() && !QGuiApplication::styleHints()->showIsMaximized()) {
        const QRect availableGeometry = dialog.screen()->availableGeometry();
        dialog.resize(availableGeometry.width() / 3, availableGeometry.height() * 2 / 3);
        dialog.move((availableGeometry.width() - dialog.width()) / 2,
                    (availableGeometry.height() - dialog.height()) / 2);
    }
    dialog.show();

    return app.exec();
}


