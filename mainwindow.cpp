#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QTimer>
#include <QRadioButton>
#include <QPushButton>
#include <QKeySequence>
#include <QScreen>
#include <QGuiApplication>
#include <QButtonGroup>
#include <QPaintEvent>
#include <QDate>
#include <QDateEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , tickCount(0)
    , timerRunning(false)
    , paintCount(0)
{
    ui->setupUi(this);

    // центрируем окно на экране
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        const QRect screenGeometry = screen->availableGeometry();
        move(screenGeometry.center() - rect().center());
    }

    // включаем трекинг мыши
    setMouseTracking(true);
    enableMouseTrackingRecursive(this);
    qApp->installEventFilter(this);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onTimerTick);

    // обновляем счётчик перерисовок
    QTimer *paintTimer = new QTimer(this);
    connect(paintTimer, &QTimer::timeout, this, [this]() {
        static int lastLogged = -1;
        if (lastLogged != paintCount) {
            ui->labelPaintCount->setText(QString("Перерисовок окна: %1").arg(paintCount));
            lastLogged = paintCount;
        }
    });
    paintTimer->start(500);

    // меню
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onActionAboutTriggered);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onActionExitTriggered);
    connect(ui->actionClear, &QAction::triggered, this, &MainWindow::onActionClearTriggered);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::onActionResetTriggered);

    // сигналы, слоты
    connect(ui->pushButtonMain, &QPushButton::clicked, this, &MainWindow::onMainButtonClicked);

    connect(ui->sliderValue, &QSlider::valueChanged, this, &MainWindow::onSliderValueChanged);
    connect(ui->spinBoxValue, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSpinBoxValueChanged);

    connect(ui->checkBoxEnable, &QCheckBox::stateChanged, this, &MainWindow::onCheckToggled);

    connect(ui->radioButtonA, &QRadioButton::toggled, this, &MainWindow::onRadioToggled);
    connect(ui->radioButtonB, &QRadioButton::toggled, this, &MainWindow::onRadioToggled);

    // группа радиокнопок
    QButtonGroup *radioGroup = new QButtonGroup(this);
    radioGroup->addButton(ui->radioButtonA);
    radioGroup->addButton(ui->radioButtonB);

    connect(ui->comboBoxItems, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onComboChanged);

    connect(ui->listWidgetItems, &QListWidget::currentRowChanged, this, &MainWindow::onListChanged);

    connect(ui->dialValue, &QDial::valueChanged, this, &MainWindow::onDialChanged);

    connect(ui->scrollBarValue, &QScrollBar::valueChanged, this, &MainWindow::onScrollChanged);

    connect(ui->tabWidgetMain, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // один обработчик на несколько кнопок
    connect(ui->pushButtonColor1, &QPushButton::clicked, this, &MainWindow::onColorButtonClicked);
    connect(ui->pushButtonColor2, &QPushButton::clicked, this, &MainWindow::onColorButtonClicked);
    connect(ui->pushButtonColor3, &QPushButton::clicked, this, &MainWindow::onColorButtonClicked);

    // программный вызов обработчика/события
    connect(ui->pushButtonCallHandler, &QPushButton::clicked, this, &MainWindow::onCallHandlerClicked);
    connect(ui->pushButtonCallEvent, &QPushButton::clicked, this, &MainWindow::onCallEventClicked);

    // таймер
    connect(ui->pushButtonTimerToggle, &QPushButton::clicked, this, &MainWindow::onTimerToggleClicked);

    // динамическое создание/уничтожение элементов
    connect(ui->pushButtonDeleteDynamic, &QPushButton::clicked, this, &MainWindow::onDeleteDynamicClicked);

    connect(ui->dateEditValue, &QDateEdit::dateChanged, this, &MainWindow::onDateChanged);

    // начальное состояние метки по чекбоксу
    onCheckToggled(ui->checkBoxEnable->isChecked() ? Qt::Checked : Qt::Unchecked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// дополнительные:

//трекинг мыши
void MainWindow::enableMouseTrackingRecursive(QWidget *widget)
{
    if (!widget)
        return;

    widget->setMouseTracking(true);
    const QList<QWidget*> children = widget->findChildren<QWidget*>();
    for (QWidget *child : children)
        child->setMouseTracking(true);
}

// строка в журнал событий и прокрутка
void MainWindow::logDebug(const QString &text)
{
    ui->listWidgetDebugLog->addItem(text);
    ui->listWidgetDebugLog->scrollToBottom();
}

// меню (верхнее)

void MainWindow::onActionAboutTriggered()
{
    QMessageBox::information(this, "О программе", "Лабораторная работа №1, Богданова Мария ТОП-204Б");
    logDebug("Меню: О программе");
}

void MainWindow::onActionExitTriggered()
{
    logDebug("Меню: Выход");
    close();
}

void MainWindow::onActionClearTriggered()
{
    ui->lineEditInput->clear();
    ui->textEditNotes->clear();
    logDebug("Меню: очищены поля ввода");
}

void MainWindow::onActionResetTriggered()
{
    ui->sliderValue->setValue(0);
    ui->spinBoxValue->setValue(0);
    ui->dialValue->setValue(0);
    ui->scrollBarValue->setValue(0);

    // убираем подсветку со всех трёх кнопок
    ui->pushButtonColor1->setStyleSheet("");
    ui->pushButtonColor2->setStyleSheet("");
    ui->pushButtonColor3->setStyleSheet("");

    logDebug("Меню: регуляторы сброшены, кнопки разблокированы");
}

// сигналы/слоты

void MainWindow::onMainButtonClicked()
{
    ui->labelClickInfo->setText("Клик выполнен");

    int v = ui->progressBarMain->value() + 10;
    if (v > 100)
        v = 0;
    ui->progressBarMain->setValue(v);

    logDebug("Нажата кнопка «Нажми меня»");
}

// слайдер и спинбокс синхронизированы в обе стороны;
// blockSignals нужен, чтобы не было рекурсии
void MainWindow::onSliderValueChanged(int value)
{
    if (ui->spinBoxValue->value() != value) {
        ui->spinBoxValue->blockSignals(true);
        ui->spinBoxValue->setValue(value);
        ui->spinBoxValue->blockSignals(false);
    }
}

void MainWindow::onSpinBoxValueChanged(int value)
{
    if (ui->sliderValue->value() != value) {
        ui->sliderValue->blockSignals(true);
        ui->sliderValue->setValue(value);
        ui->sliderValue->blockSignals(false);
    }
}

void MainWindow::onCheckToggled(int state)
{
    ui->labelCheckBox->setVisible(state == Qt::Checked);
    logDebug(QString("Чекбокс: %1").arg(state == Qt::Checked ? "включён" : "выключен"));
}

// один слот на две радиокнопки, определяем нажатую через sender
void MainWindow::onRadioToggled(bool checked)
{
    if (!checked)
        return;

    QRadioButton *rb = qobject_cast<QRadioButton*>(sender());
    if (rb) {
        ui->labelRadio->setText("Выбран вариант: " + rb->text());
        logDebug("Радиокнопка: " + rb->text());
    }
}

void MainWindow::onComboChanged(int index)
{
    ui->labelCombo->setText("Выбрано: " + ui->comboBoxItems->itemText(index));
    logDebug("Комбобокс: " + ui->comboBoxItems->itemText(index));
}

void MainWindow::onListChanged(int row)
{
    if (row >= 0) {
        ui->labelList->setText("Выбран элемент: " + ui->listWidgetItems->item(row)->text());
        logDebug("Список: " + ui->listWidgetItems->item(row)->text());
    } else {
        ui->labelList->setText("Выбран элемент: нет");
    }
}

void MainWindow::onDialChanged(int value)
{
    ui->lcdNumberDisplay->display(value);
}

void MainWindow::onScrollChanged(int value)
{
    ui->progressBarScroll->setValue(value);
}

void MainWindow::onTabChanged(int index)
{
    ui->labelTab->setText(QString("Активная вкладка: %1").arg(index + 1));
    logDebug(QString("Переключена вкладка: %1").arg(index + 1));
}

// один обработчик на несколько кнопок

void MainWindow::onColorButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn)
        return;

    ui->pushButtonColor1->setStyleSheet("");
    ui->pushButtonColor2->setStyleSheet("");
    ui->pushButtonColor3->setStyleSheet("");

    btn->setStyleSheet("background-color: yellow");

    logDebug("Нажата кнопка: " + btn->text());
}

// вызов обработчика и события из кода

void MainWindow::onCallHandlerClicked()
{
    // вызываем обработчик как обычный метод
    onMainButtonClicked();
    logDebug("Программный вызов обработчика (прямой вызов метода)");
}

void MainWindow::onCallEventClicked()
{
    // клик по кнопке программно
    logDebug("Программный вызов события (pushButtonMain->click())");
    ui->pushButtonMain->click();
}

// таймер

void MainWindow::onTimerToggleClicked()
{
    if (timerRunning) {
        timer->stop();
        timerRunning = false;
        ui->pushButtonTimerToggle->setText("Старт таймера");
        logDebug("Таймер остановлен");
    } else {
        timer->start(1000);
        timerRunning = true;
        ui->pushButtonTimerToggle->setText("Стоп таймера");
        logDebug("Таймер запущен");
    }
}

void MainWindow::onTimerTick()
{
    tickCount++;
    ui->labelTimerTicks->setText(QString("Секунд: %1").arg(tickCount));

    int v = ui->progressBarTimer->value() + 10;
    if (v > 100)
        v = 0;
    ui->progressBarTimer->setValue(v);

    logDebug(QString("Тик таймера: %1").arg(tickCount));
}

// события мыши, клавиатуры и окна

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    const QString text = QString("Мышь нажата: (%1, %2)").arg(event->pos().x()).arg(event->pos().y());
    ui->labelMousePress->setText(text);
    logDebug(text);
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    const QString text = QString("Мышь отпущена: (%1, %2)").arg(event->pos().x()).arg(event->pos().y());
    ui->labelMousePress->setText(text);
    logDebug(text);
    QMainWindow::mouseReleaseEvent(event);
}

// фильтр событий на уровне приложения
// ловит движение мыши по всему окну и клавиши независимо от фокуса, а также клики по области для создания кнопок
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        QPoint posInWindow = mapFromGlobal(me->globalPos());
        ui->labelMouseMove->setText(
            QString("Позиция мыши: (%1, %2)").arg(posInWindow.x()).arg(posInWindow.y()));
    } else if (event->type() == QEvent::KeyPress && watched == this) {
        handleKeyEvent(static_cast<QKeyEvent*>(event), true);
    } else if (event->type() == QEvent::KeyRelease && watched == this) {
        handleKeyEvent(static_cast<QKeyEvent*>(event), false);
    } else if (event->type() == QEvent::MouseButtonPress && watched == ui->dynamicArea) {
        // клик по свободной области - создаём кнопку
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        createDynamicButton(me->pos());
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    // Считаем перерисовки
    paintCount++;
    QMainWindow::paintEvent(event);
}

// получаем читаемое имя клавиши
void MainWindow::handleKeyEvent(QKeyEvent *event, bool pressed)
{
    QString keyName = QKeySequence(event->key()).toString(QKeySequence::NativeText);
    if (keyName.isEmpty())
        keyName = QString("код %1").arg(event->key());
    const QString text = (pressed ? "Клавиша нажата: " : "Клавиша отпущена: ") + keyName;
    ui->labelKey->setText(text);
    logDebug(text);
}

// изменение размера
void MainWindow::resizeEvent(QResizeEvent *event)
{
    ui->labelResize->setText(QString("Размер окна: %1 x %2").arg(width()).arg(height()));
    logDebug(QString("Изменён размер окна: %1 x %2").arg(width()).arg(height()));
    QMainWindow::resizeEvent(event);
}

// создаём кнопку в точке клика внутри dynamic area и подписываем на слот

void MainWindow::createDynamicButton(const QPoint &pos)
{
    QPushButton *btn = new QPushButton(QString("Кнопка %1").arg(dynamicButtons.size() + 1), ui->dynamicArea);
    btn->move(pos.x() - btn->sizeHint().width() / 2, pos.y() - btn->sizeHint().height() / 2);
    btn->show();

    // все динамические кнопки используют один слот
    connect(btn, &QPushButton::clicked, this, &MainWindow::onDynamicButtonClicked);

    dynamicButtons.append(btn);
    ui->labelDynamicInfo->setText(QString("Кнопок создано: %1").arg(dynamicButtons.size()));
    logDebug(QString("Создана динамическая кнопка: %1").arg(btn->text()));
}

void MainWindow::onDynamicButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (btn)
        logDebug("Нажата динамическая кнопка: " + btn->text());
}

void MainWindow::onDeleteDynamicClicked()
{
    if (dynamicButtons.isEmpty())
        return;

    QPushButton *btn = dynamicButtons.takeLast();
    logDebug("Удалена динамическая кнопка: " + btn->text());
    delete btn;

    ui->labelDynamicInfo->setText(QString("Кнопок создано: %1").arg(dynamicButtons.size()));
}

void MainWindow::onDateChanged(const QDate &date)
{
    ui->labelCaptionDate->setText("Дата: " + date.toString("dd.MM.yyyy"));
}
