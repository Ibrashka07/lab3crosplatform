#include "colorwidget.h"

#include <QDebug>           // qDebug() — вывод в "Вывод приложения"
#include <QMouseEvent>      // QMouseEvent — событие мыши
#include <QPainter>         // QPainter — рисование
#include <QRandomGenerator> // генератор случайных чисел Qt

// ============================================================
//  КОНСТРУКТОР / ДЕСТРУКТОР
// ============================================================
ColorWidget::ColorWidget(QWidget *parent)
    : QWidget(parent)
    , m_palette(defaultPalette())   // стартовая палитра
    , m_index(0)                    // начинаем с первого цвета
    , m_blinkOn(false)              // мигание выключено
    , m_blinkBackup(Qt::black)      // запасной цвет (пока не используется)
{
    // Логирование в конструкторе/деструкторе показывает жизненный цикл
    // объекта (связь с ЛР2 про дерево объектов).
    qDebug() << "[CREATE] ColorWidget, палитра:" << m_palette.size();
}

ColorWidget::~ColorWidget()
{
    qDebug() << "[DESTROY] ColorWidget";
}

// ============================================================
//  ПАЛИТРА ПО УМОЛЧАНИЮ
// ============================================================
QVector<QColor> ColorWidget::defaultPalette()
{
    // QColor можно задавать HEX-строкой вида "#rrggbb".
    return {
        QColor("#e74c3c"), QColor("#e67e22"), QColor("#f1c40f"),
        QColor("#2ecc71"), QColor("#3498db"), QColor("#9b59b6")
    };
}

// ============================================================
//  ТЕКУЩИЙ ЦВЕТ
// ============================================================
QColor ColorWidget::currentColor() const
{
    if (m_palette.isEmpty())
        return Qt::gray;   // защита от пустой палитры
    // m_index % size защищает от выхода за границы, даже если индекс «сбился».
    return m_palette.at(m_index % m_palette.size());
}

// ============================================================
//  ДОБАВЛЕНИЕ ЦВЕТА
// ============================================================
void ColorWidget::addColor(const QColor &c)
{
    m_palette.push_back(c);
    qDebug() << "[PALETTE] Добавлен цвет" << c.name()
             << ", всего:" << m_palette.size();

    // update() НЕ перерисовывает сразу — он лишь ставит в очередь
    // запрос на перерисовку. Реальный paintEvent вызовет цикл событий.
    update();
}

// ============================================================
//  СБРОС ПАЛИТРЫ
// ============================================================
void ColorWidget::resetPalette()
{
    m_palette = defaultPalette();
    m_index   = 0;
    m_blinkOn = false;
    update();
    emitCurrent();  // сообщить миру, что цвет изменился
}

// ============================================================
//  ШАГ ВПЕРЁД / НАЗАД
// ============================================================
void ColorWidget::stepForward()
{
    if (m_palette.isEmpty()) return;
    // (index + 1) % size — переход по кругу: после последнего идёт первый.
    m_index = (m_index + 1) % m_palette.size();
    m_blinkOn = false;
    update();
    emitCurrent();
}

void ColorWidget::stepBackward()
{
    if (m_palette.isEmpty()) return;
    // + size — чтобы избежать отрицательного остатка в C++
    // (в C++ -1 % 6 == -1, а не 5, как в математике).
    m_index = (m_index - 1 + m_palette.size()) % m_palette.size();
    m_blinkOn = false;
    update();
    emitCurrent();
}

// ============================================================
//  СЛУЧАЙНЫЙ ЦВЕТ
// ============================================================
void ColorWidget::setRandomColor()
{
    if (m_palette.isEmpty()) return;
    // bounded(n) даёт случайное в [0, n). Безопаснее, чем rand() % n.
    m_index = QRandomGenerator::global()->bounded(m_palette.size());
    m_blinkOn = false;
    update();
    emitCurrent();
}

// ============================================================
//  МИГАНИЕ
// ============================================================
void ColorWidget::toggleBlink()
{
    if (m_blinkOn) {
        // Выключаем мигание: paintEvent снова нарисует currentColor().
        m_blinkOn = false;
    } else {
        // Включаем: запомнили текущий цвет, флаг заставит рисовать чёрный.
        m_blinkBackup = currentColor();
        m_blinkOn = true;
    }
    update();
    emitCurrent();
}

// ============================================================
//  ИЗЛУЧЕНИЕ СИГНАЛА
// ============================================================
void ColorWidget::emitCurrent()
{
    // Если мигаем — внешнему миру сообщаем «чёрный»,
    // иначе — текущий цвет палитры.
    const QColor c = m_blinkOn ? QColor(0, 0, 0) : currentColor();
    emit colorChanged(c);   // emit — просто ключевое слово для читаемости
}

// ============================================================
//  ОТРИСОВКА
// ============================================================
void ColorWidget::paintEvent(QPaintEvent * /*event*/)
{
    // QPainter — «кисть» для рисования на этом виджете.
    QPainter p(this);

    // Какой цвет заливать: чёрный при мигании или текущий.
    const QColor fill = m_blinkOn ? QColor(0, 0, 0) : currentColor();

    // Заливаем весь прямоугольник виджета.
    p.fillRect(rect(), fill);

    // Контрастный текст: на светлом фоне — чёрный, на тёмном — белый.
    // lightness() возвращает 0..255.
    p.setPen(fill.lightness() > 128 ? Qt::black : Qt::white);

    // Рисуем HEX-имя цвета по центру.
    p.drawText(rect(), Qt::AlignCenter, fill.name());
}

// ============================================================
//  МЫШЬ
// ============================================================
void ColorWidget::mousePressEvent(QMouseEvent *event)
{
    // event->button() — КАКАЯ кнопка вызвала это событие (левая/правая).
    // (Не путать с event->buttons() — какие кнопки зажаты в момент события.)
    if (event->button() == Qt::LeftButton) {
        qDebug() << "[MOUSE] Левый клик по холсту";
        stepForward();
    } else if (event->button() == Qt::RightButton) {
        qDebug() << "[MOUSE] Правый клик по холсту";
        stepBackward();
    }

    // Обязательно передаём событие базовому классу:
    // это стандартная практика, иначе можем сломать поведение QWidget.
    QWidget::mousePressEvent(event);
}