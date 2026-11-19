#include "mainwindow.h"

#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>      // нужен: setupUi() создаёт new QHBoxLayout()
#include <QLabel>
#include <QPushButton>      // нужен: setupUi() создаёт new QPushButton()
#include <QRandomGenerator>
#include <QVBoxLayout>

// ============================================================
//  КОНСТРУКТОР / ДЕСТРУКТОР
// ============================================================
MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , m_mode(Mode::Cycle)        // стартовый режим — цикл
    , m_clickHandlerAlt(false)   // стартовый обработчик — основной
{
    setupUi();                    // строим интерфейс кодом

    // Настраиваем таймер, но НЕ запускаем — пусть пользователь нажмёт «Старт».
    m_timer.setInterval(600);

    // Создаём первоначальные соединения.
    rebuildTimeoutConnection();   // timeout -> слот по режиму
    reconnectCanvasHandler();     // colorChanged -> обработчик статуса

    setStatus(QStringLiteral("Готово. Нажмите «Старт/Стоп», чтобы запустить таймер."));
}

MainWindow::~MainWindow()
{
    qDebug() << "[DESTROY] MainWindow";
    m_timer.stop();   // вежливо останавливаем таймер перед уничтожением
}

// ============================================================
//  ПОСТРОЕНИЕ ИНТЕРФЕЙСА КОДОМ
// ============================================================
void MainWindow::setupUi()
{
    setWindowTitle(QStringLiteral("ColorSwitcher — ЛР3"));

    // Корневой вертикальный слой (компоновщик).
    m_rootLayout = new QVBoxLayout(this);

    // ---- Холст ----
    m_canvas.setMinimumHeight(260);
    // stretch=1 => холст занимает всё свободное место при растяжении окна.
    m_rootLayout->addWidget(&m_canvas, 1);

    // ---- Подписи интервал/режим ----
    m_intervalLabel = new QLabel(this);
    m_modeLabel     = new QLabel(this);
    m_statusLabel   = new QLabel(this);

    // Горизонтальный ряд для двух подписей.
    auto *infoRow = new QHBoxLayout();
    infoRow->addWidget(m_intervalLabel);
    infoRow->addStretch(1);           // «пружина» раздвигает подписи
    infoRow->addWidget(m_modeLabel);
    m_rootLayout->addLayout(infoRow);

    // ---- Кнопки управления ----
    auto *btnStartStop = new QPushButton(QStringLiteral("Старт/Стоп"), this);
    auto *btnMinus     = new QPushButton(QStringLiteral("− интервал"), this);
    auto *btnPlus      = new QPushButton(QStringLiteral("+ интервал"), this);
    auto *btnMode      = new QPushButton(QStringLiteral("Режим"), this);
    auto *btnHandler   = new QPushButton(QStringLiteral("Сменить обработчик клика"), this);
    auto *btnAddColor  = new QPushButton(QStringLiteral("+ цвет"), this);
    auto *btnClear     = new QPushButton(QStringLiteral("Очистить цвета"), this);

    auto *ctrlRow = new QHBoxLayout();
    ctrlRow->addWidget(btnStartStop);
    ctrlRow->addWidget(btnMinus);
    ctrlRow->addWidget(btnPlus);
    ctrlRow->addWidget(btnMode);
    ctrlRow->addWidget(btnHandler);
    ctrlRow->addStretch(1);
    ctrlRow->addWidget(btnAddColor);
    ctrlRow->addWidget(btnClear);
    m_rootLayout->addLayout(ctrlRow);

    // ---- Сетка для динамических кнопок-цветов ----
    m_paletteGrid = new QGridLayout();
    m_rootLayout->addLayout(m_paletteGrid);

    // ---- Статусная строка снизу ----
    m_rootLayout->addWidget(m_statusLabel);

    // ---- Классические connect: сигнал кнопки -> слот класса ----
    // Здесь третий аргумент this — контекст: если MainWindow умрёт,
    // Qt разорвёт соединение автоматически.
    connect(btnStartStop, &QPushButton::clicked, this, &MainWindow::toggleTimer);
    connect(btnMinus,     &QPushButton::clicked, this, &MainWindow::decreaseInterval);
    connect(btnPlus,      &QPushButton::clicked, this, &MainWindow::increaseInterval);
    connect(btnMode,      &QPushButton::clicked, this, &MainWindow::cycleMode);
    connect(btnHandler,   &QPushButton::clicked, this, &MainWindow::toggleClickHandler);
    connect(btnAddColor,  &QPushButton::clicked, this, &MainWindow::addColorButton);
    connect(btnClear,     &QPushButton::clicked, this, &MainWindow::clearPaletteButtons);

    updateIntervalLabel();
    updateModeLabel();
}

// ============================================================
//  ДИНАМИЧЕСКИЕ СОЕДИНЕНИЯ: ТАЙМЕР
// ============================================================
void MainWindow::rebuildTimeoutConnection()
{
    // 1) Если соединение уже было — рвём его и обнуляем идентификатор.
    if (m_timeoutConn) {
        disconnect(m_timeoutConn);
        m_timeoutConn = QMetaObject::Connection();
    }

    // 2) Создаём новое соединение под текущий режим.
    //    Лямбда захватывает [this] — стандарт для слотов класса.
    //    Третий аргумент (this) — контекст: защита от use-after-free.
    switch (m_mode) {
    case Mode::Cycle:
        m_timeoutConn = connect(&m_timer, &QTimer::timeout, this, [this]() {
            m_canvas.stepForward();
        });
        break;
    case Mode::Random:
        m_timeoutConn = connect(&m_timer, &QTimer::timeout, this, [this]() {
            m_canvas.setRandomColor();
        });
        break;
    case Mode::Blink:
        m_timeoutConn = connect(&m_timer, &QTimer::timeout, this, [this]() {
            m_canvas.toggleBlink();
        });
        break;
    }
}

// ============================================================
//  ДИНАМИЧЕСКИЕ СОЕДИНЕНИЯ: СИГНАЛ ХОЛСТА
// ============================================================
void MainWindow::reconnectCanvasHandler()
{
    // Рвём предыдущее соединение, если оно было.
    if (m_clickConn) {
        disconnect(m_clickConn);
        m_clickConn = QMetaObject::Connection();
    }

    // Лямбда принимает const QColor& — сигнал colorChanged передаёт цвет.
    if (!m_clickHandlerAlt) {
        // Основной обработчик: просто пишем статус.
        m_clickConn = connect(&m_canvas, &ColorWidget::colorChanged,
                              this, [this](const QColor &c) {
                                  setStatus(QStringLiteral("Цвет: %1").arg(c.name()));
                              });
    } else {
        // Альтернативный: пишем статус И меняем фон главного окна.
        m_clickConn = connect(&m_canvas, &ColorWidget::colorChanged,
                              this, [this](const QColor &c) {
                                  setStatus(QStringLiteral("[ALT] Цвет: %1 (фон окна обновлён)")
                                                .arg(c.name()));
                                  setStyleSheet(QStringLiteral("MainWindow { background: %1; }")
                                                    .arg(c.name()));
                              });
    }
}

// ============================================================
//  СТАРТ / СТОП ТАЙМЕРА
// ============================================================
void MainWindow::toggleTimer()
{
    if (m_timer.isActive()) {
        m_timer.stop();
        setStatus(QStringLiteral("Таймер остановлен."));
    } else {
        // start() без аргументов запускает ПОВТОРЯЮЩИЙСЯ таймер
        // с текущим interval().
        m_timer.start();
        setStatus(QStringLiteral("Таймер запущен, интервал %1 мс.")
                      .arg(m_timer.interval()));
    }
}

// ============================================================
//  ИНТЕРВАЛ
// ============================================================
void MainWindow::decreaseInterval()
{
    // Не ниже 100 мс — иначе интерфейс «зальётся» событиями.
    m_timer.setInterval(qMax(100, m_timer.interval() - 100));
    updateIntervalLabel();
}

void MainWindow::increaseInterval()
{
    m_timer.setInterval(m_timer.interval() + 100);
    updateIntervalLabel();
}

// ============================================================
//  РЕЖИМЫ
// ============================================================
void MainWindow::cycleMode()
{
    // Циклический переход по трём режимам.
    switch (m_mode) {
    case Mode::Cycle:  m_mode = Mode::Random; break;
    case Mode::Random: m_mode = Mode::Blink;  break;
    case Mode::Blink:  m_mode = Mode::Cycle;  break;
    }

    // Пересоздаём соединение таймера, чтобы новый режим заработал.
    rebuildTimeoutConnection();
    updateModeLabel();
    setStatus(QStringLiteral("Режим: %1").arg(modeName()));
}

QString MainWindow::modeName() const
{
    switch (m_mode) {
    case Mode::Cycle:  return QStringLiteral("Цикл");
    case Mode::Random: return QStringLiteral("Случайный");
    case Mode::Blink:  return QStringLiteral("Мигание");
    }
    return QStringLiteral("?");
}

void MainWindow::updateModeLabel()
{
    m_modeLabel->setText(QStringLiteral("Режим: %1").arg(modeName()));
}

void MainWindow::updateIntervalLabel()
{
    m_intervalLabel->setText(QStringLiteral("Интервал: %1 мс")
                                 .arg(m_timer.interval()));
}

// ============================================================
//  ДИНАМИЧЕСКИЕ КНОПКИ-ЦВЕТА
// ============================================================
void MainWindow::addColorButton()
{
    // Случайный цвет.
    const int r = QRandomGenerator::global()->bounded(256);
    const int g = QRandomGenerator::global()->bounded(256);
    const int b = QRandomGenerator::global()->bounded(256);
    const QColor c(r, g, b);

    // Кнопка создаётся В РАНТАЙМЕ. Родитель this — Qt удалит её
    // при уничтожении MainWindow (дерево объектов, ЛР2).
    auto *btn = new PaletteButton(c, this);

    // Размещаем в сетке по 4 в ряд.
    const int idx = m_paletteButtons.size();
    m_paletteGrid->addWidget(btn, idx / 4, idx % 4);

    // ДИНАМИЧЕСКОЕ СОЕДИНЕНИЕ.
    // Захват [this, btn] ПО ЗНАЧЕНИЮ: это безопасно (оба указателя живы,
    // пока соединение активно).
    // Антипаттерн [&] здесь был бы опасен: btn — локальная переменная,
    // после выхода из функции ссылка стала бы висячей.
    QMetaObject::Connection conn = connect(
        btn, &QPushButton::clicked, this,
        [this, btn]() {
            m_canvas.addColor(btn->color());
            setStatus(QStringLiteral("Добавлен цвет %1").arg(btn->color().name()));
        });

    // Запоминаем кнопку и её соединение, чтобы потом убрать.
    m_paletteButtons.append(QPointer<PaletteButton>(btn));
    m_dynamicConnections.append(conn);

    setStatus(QStringLiteral("Создана кнопка-цвет %1").arg(c.name()));
}

void MainWindow::clearPaletteButtons()
{
    // 1) Явно разрываем все динамические соединения.
    for (const auto &conn : m_dynamicConnections) {
        if (conn) disconnect(conn);
    }
    m_dynamicConnections.clear();

    // 2) Удаляем сами кнопки.
    // deleteLater() безопаснее delete: объект удалится в конце текущей
    // итерации цикла событий, а не прямо сейчас (внутри обработчика клика).
    for (auto &p : m_paletteButtons) {
        if (p) p->deleteLater();
    }
    m_paletteButtons.clear();

    setStatus(QStringLiteral("Все кнопки-цвета удалены и соединения разорваны."));
}

// ============================================================
//  ПЕРЕКЛЮЧЕНИЕ ОБРАБОТЧИКА СИГНАЛА ХОЛСТА
// ============================================================
void MainWindow::toggleClickHandler()
{
    m_clickHandlerAlt = !m_clickHandlerAlt;
    reconnectCanvasHandler();   // пересоздаём соединение с другой лямбдой
    setStatus(QStringLiteral("Обработчик клика: %1")
                  .arg(m_clickHandlerAlt ? "ALT" : "основной"));
}

// ============================================================
//  СТАТУС
// ============================================================
void MainWindow::setStatus(const QString &text)
{
    m_statusLabel->setText(text);
    qDebug() << "[STATUS]" << text;
}