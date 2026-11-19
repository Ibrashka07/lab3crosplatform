#include "palettebutton.h"
#include <QDebug>

// Конструктор: сохраняет цвет и настраивает внешний вид кнопки.
PaletteButton::PaletteButton(const QColor &color, QWidget *parent)
    : QPushButton(parent)
    , m_color(color)
{
    // Надпись на кнопке — её HEX-имя (например, "#3498db").
    setText(color.name());

    // Подсказка при наведении мыши.
    setToolTip(QStringLiteral("Добавить этот цвет в палитру холста"));

    // Логирование жизненного цикла (связь с ЛР2).
    qDebug() << "[CREATE] PaletteButton" << color.name();
}

// Деструктор — чтобы в логе было видно, когда кнопка реально удалена.
PaletteButton::~PaletteButton()
{
    qDebug() << "[DESTROY] PaletteButton" << m_color.name();
}