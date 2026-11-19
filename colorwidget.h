#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include <QColor>
#include <QVector>
#include <QWidget>

// ColorWidget — «холст». Его задачи:
//  1. Хранить палитру цветов и текущий индекс.
//  2. Рисовать себя текущим цветом (paintEvent).
//  3. Реагировать на клики мыши (mousePressEvent).
//  4. Излучать сигнал colorChanged(), чтобы внешний мир знал об изменениях.
class ColorWidget : public QWidget
{
    // Q_OBJECT — макрос, который moc превратит в метаобъект.
    // Без него сигнал colorChanged не будет работать: connect не найдёт его.
    Q_OBJECT

public:
    explicit ColorWidget(QWidget *parent = nullptr);
    ~ColorWidget() override;

    void   addColor(const QColor &c);   // добавить цвет в конец палитры
    void   resetPalette();              // сброс к палитре по умолчанию
    QColor currentColor() const;        // текущий цвет (для внешнего мира)

    void stepForward();                 // следующий цвет (обычно левый клик)
    void stepBackward();                // предыдущий (обычно правый клик)
    void setRandomColor();              // случайный цвет
    void toggleBlink();                 // переключить «мигание» (чёрный/цвет)

signals:
    // Собственный сигнал: «цвет изменился».
    // Это мост между низкоуровневым событием (мышь) и высокоуровневым
    // уведомлением для других объектов. Сигналы — не виртуальные функции,
    // они генерируются moc и работают через метасистему Qt.
    void colorChanged(const QColor &color);

protected:
    // Переопределения виртуальных методов QWidget.
    // Их вызывает САМА Qt, когда приходит событие от ОС.
    // К ним нельзя подключиться через connect() — это не сигналы.
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QVector<QColor> m_palette;    // вся палитра
    int             m_index;      // индекс текущего цвета в палитре
    bool            m_blinkOn;    // включён ли режим мигания
    QColor          m_blinkBackup;// запомненный цвет (на будущее)

    static QVector<QColor> defaultPalette();  // палитра по умолчанию
    void emitCurrent();                       // издать сигнал с текущим цветом
};

#endif // COLORWIDGET_H