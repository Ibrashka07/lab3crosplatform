#ifndef PALETTEBUTTON_H
#define PALETTEBUTTON_H

#include <QColor>
#include <QPushButton>

// PaletteButton — маленькая кнопка, которая помнит свой цвет.
// Такие кнопки создаются в рантайме и подключаются динамически
// (см. MainWindow::addColorButton).
class PaletteButton : public QPushButton
{
    Q_OBJECT

public:
    explicit PaletteButton(const QColor &color, QWidget *parent = nullptr);
    ~PaletteButton() override;

    // Геттер. Тело внутри класса => неявный inline — легально
    // и не вызовет multiple definition, даже если .h включён в нескольких .cpp.
    QColor color() const { return m_color; }

private:
    QColor m_color;   // цвет, который кнопка «добавляет» в палитру
};

#endif // PALETTEBUTTON_H