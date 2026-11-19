#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // QApplication создаёт цикл событий — без него таймер не тикает.
    QApplication app(argc, argv);

    // Главное окно живёт до выхода из main().
    MainWindow w;
    w.resize(800, 600);   // начальный размер окна
    w.show();

    // КЛЮЧЕВОЕ: app.exec() запускает цикл событий.
    // Без него окно отрисуется один раз и "замрёт".
    return app.exec();
}