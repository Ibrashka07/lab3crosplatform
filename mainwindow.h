#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMetaObject>   // QMetaObject::Connection — идентификатор соединения
#include <QPointer>      // QPointer — «безопасный» указатель на QObject
#include <QTimer>        // QTimer — как ПОЛЕ класса (по значению)
#include <QWidget>

#include "colorwidget.h"    // нужен полный тип (поле по значению)
#include "palettebutton.h"  // используется в QPointer<PaletteButton>

// Предварительные объявления: этих классов в .h достаточно знать по имени.
// Полный #include нужен только в .cpp, где мы их создаём.
class QLabel;
class QVBoxLayout;
class QGridLayout;

// MainWindow — «дирижёр». Связывает холст, таймер и кнопки управления.
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // Слоты, которые подключены к сигналам кнопок.
    void toggleTimer();            // Старт/Стоп таймера
    void decreaseInterval();       // - интервал
    void increaseInterval();       // + интервал
    void cycleMode();              // Cycle -> Random -> Blink -> Cycle
    void addColorButton();         // создать кнопку-цвет
    void clearPaletteButtons();    // удалить кнопки и разорвать соединения
    void toggleClickHandler();     // переподключить colorChanged

private:
    enum class Mode { Cycle, Random, Blink };  // три режима работы таймера

    // Вспомогательные методы (НЕ слоты).
    void setupUi();                  // создать интерфейс кодом
    void rebuildTimeoutConnection(); // пересоздать соединение timeout -> лямбда
    void reconnectCanvasHandler();   // пересоздать соединение colorChanged -> лямбда
    void setStatus(const QString &text);
    void updateIntervalLabel();
    void updateModeLabel();
    QString modeName() const;

    // ------- Данные -------
    QTimer      m_timer;            // сам таймер (объект-поле)
    ColorWidget m_canvas;           // холст
    Mode        m_mode;             // текущий режим
    bool        m_clickHandlerAlt;  // какой обработчик colorChanged активен

    // Идентификаторы соединений — чтобы можно было их РВАТЬ вручную.
    // (Теория: connect() возвращает QMetaObject::Connection.)
    QMetaObject::Connection m_timeoutConn;   // timeout() -> лямбда
    QMetaObject::Connection m_clickConn;     // colorChanged() -> лямбда

    // Виджеты интерфейса.
    QVBoxLayout *m_rootLayout    = nullptr;
    QGridLayout *m_paletteGrid   = nullptr;
    QLabel      *m_statusLabel   = nullptr;
    QLabel      *m_intervalLabel = nullptr;
    QLabel      *m_modeLabel     = nullptr;

    // Динамические кнопки-цвета и их соединения.
    // QPointer автоматически становится nullptr, если объект удалён.
    QList<QPointer<PaletteButton>> m_paletteButtons;
    QList<QMetaObject::Connection> m_dynamicConnections;
};

#endif // MAINWINDOW_H