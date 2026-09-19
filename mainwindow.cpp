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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , tickCount(0)
    , timerRunning(false)
{
    ui->setupUi(this);

    // окно фиксированного размера ~1000x820, по центру экрана
    if (QScreen *screen = QGuiApplication::primaryScreen()) {
        const QRect screenGeometry = screen->availableGeometry();
        move(screenGeometry.center() - rect().center());
    }

    // включаем отслеживание мыши на самом окне и на всех дочерних виджетах,
    // иначе перемещение мыши без нажатой кнопки не долетает до дочерних элементов.
    // тот же eventFilter на уровне приложения ловит и нажатия клавиш —
    // независимо от того, какой дочерний виджет сейчас в фокусе
    setMouseTracking(true);
    enableMouseTrackingRecursive(this);
    qApp->installEventFilter(this);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::onTimerTick);

    // --- меню ---
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onActionAboutTriggered);
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onActionExitTriggered);
    connect(ui->actionClear, &QAction::triggered, this, &MainWindow::onActionClearTriggered);
    connect(ui->actionReset, &QAction::triggered, this, &MainWindow::onActionResetTriggered);

    // --- п.5: сигналы/слоты ---
    connect(ui->pushButtonMain, &QPushButton::clicked, this, &MainWindow::onMainButtonClicked);

    connect(ui->sliderValue, &QSlider::valueChanged, this, &MainWindow::onSliderValueChanged);
    connect(ui->spinBoxValue, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onSpinBoxValueChanged);

    connect(ui->checkBoxEnable, &QCheckBox::stateChanged, this, &MainWindow::onCheckToggled);

    connect(ui->radioButtonA, &QRadioButton::toggled, this, &MainWindow::onRadioToggled);
    connect(ui->radioButtonB, &QRadioButton::toggled, this, &MainWindow::onRadioToggled);

    connect(ui->comboBoxItems, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onComboChanged);

    connect(ui->listWidgetItems, &QListWidget::currentRowChanged, this, &MainWindow::onListChanged);

    connect(ui->dialValue, &QDial::valueChanged, this, &MainWindow::onDialChanged);

    connect(ui->scrollBarValue, &QScrollBar::valueChanged, this, &MainWindow::onScrollChanged);

    connect(ui->tabWidgetMain, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // --- п.6: один обработчик на несколько кнопок ---
    connect(ui->pushButtonColor1, &QPushButton::clicked, this, &MainWindow::onColorButtonClicked);
    connect(ui->pushButtonColor2, &QPushButton::clicked, this, &MainWindow::onColorButtonClicked);
    connect(ui->pushButtonColor3, &QPushButton::clicked, this, &MainWindow::onColorButtonClicked);

    // --- п.7: программный вызов обработчика/события ---
    connect(ui->pushButtonCallHandler, &QPushButton::clicked, this, &MainWindow::onCallHandlerClicked);
    connect(ui->pushButtonCallEvent, &QPushButton::clicked, this, &MainWindow::onCallEventClicked);

    // --- п.8: таймер ---
    connect(ui->pushButtonTimerToggle, &QPushButton::clicked, this, &MainWindow::onTimerToggleClicked);

    // начальная видимость пояснительной метки по чекбоксу
    onCheckToggled(ui->checkBoxEnable->isChecked() ? Qt::Checked : Qt::Unchecked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ===================== вспомогательные =====================

void MainWindow::enableMouseTrackingRecursive(QWidget *widget)
{
    if (!widget)
        return;

    widget->setMouseTracking(true);
    const QList<QWidget*> children = widget->findChildren<QWidget*>();
    for (QWidget *child : children)
        child->setMouseTracking(true);
}

void MainWindow::logDebug(const QString &text)
{
    ui->listWidgetDebugLog->addItem(text);
    ui->listWidgetDebugLog->scrollToBottom();
}

// ===================== меню =====================

void MainWindow::onActionAboutTriggered()
{
    QMessageBox::information(this, "Файл", "Пункт меню: О программе");
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

    // снимаем подсветку с "нажатых" кнопок (п.6)
    ui->pushButtonColor1->setStyleSheet("");
    ui->pushButtonColor2->setStyleSheet("");
    ui->pushButtonColor3->setStyleSheet("");

    logDebug("Меню: регуляторы сброшены, кнопки разблокированы");
}

// ===================== п.5: сигналы/слоты =====================

void MainWindow::onMainButtonClicked()
{
    ui->labelClickInfo->setText("Клик выполнен");

    int v = ui->progressBarMain->value() + 10;
    if (v > 100)
        v = 0;
    ui->progressBarMain->setValue(v);

    logDebug("Нажата кнопка «Нажми меня»");
}

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
}

void MainWindow::onRadioToggled(bool checked)
{
    if (!checked)
        return;

    QRadioButton *rb = qobject_cast<QRadioButton*>(sender());
    if (rb)
        ui->labelRadio->setText("Выбран вариант: " + rb->text());
}

void MainWindow::onComboChanged(int index)
{
    ui->labelCombo->setText("Выбрано: " + ui->comboBoxItems->itemText(index));
}

void MainWindow::onListChanged(int row)
{
    if (row >= 0)
        ui->labelList->setText("Выбран элемент: " + ui->listWidgetItems->item(row)->text());
    else
        ui->labelList->setText("Выбран элемент: нет");
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

// ===================== п.6: один обработчик на несколько кнопок =====================

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

// ===================== п.7: программный вызов обработчика/события =====================

void MainWindow::onCallHandlerClicked()
{
    // программный вызов обработчика: обычный вызов метода
    onMainButtonClicked();
    logDebug("Программный вызов обработчика (прямой вызов метода)");
}

void MainWindow::onCallEventClicked()
{
    // программный вызов события
    logDebug("Программный вызов события (pushButtonMain->click())");
    ui->pushButtonMain->click();
}

// ===================== п.8: таймер (со своим прогрессбаром) =====================

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
}

// ===================== п.4: события мыши/клавиатуры/окна =====================

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    ui->labelMousePress->setText(
        QString("Мышь нажата: (%1, %2)").arg(event->pos().x()).arg(event->pos().y()));
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    ui->labelMousePress->setText(
        QString("Мышь отпущена: (%1, %2)").arg(event->pos().x()).arg(event->pos().y()));
    QMainWindow::mouseReleaseEvent(event);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        QPoint posInWindow = mapFromGlobal(me->globalPos());
        ui->labelMouseMove->setText(
            QString("Позиция мыши: (%1, %2)").arg(posInWindow.x()).arg(posInWindow.y()));
    } else if (event->type() == QEvent::KeyPress) {
        handleKeyEvent(static_cast<QKeyEvent*>(event), true);
    } else if (event->type() == QEvent::KeyRelease) {
        handleKeyEvent(static_cast<QKeyEvent*>(event), false);
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::handleKeyEvent(QKeyEvent *event, bool pressed)
{
    QString keyName = QKeySequence(event->key()).toString(QKeySequence::NativeText);
    if (keyName.isEmpty())
        keyName = QString("код %1").arg(event->key());
    ui->labelKey->setText((pressed ? "Клавиша нажата: " : "Клавиша отпущена: ") + keyName);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    handleKeyEvent(event, true);
    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    handleKeyEvent(event, false);
    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    ui->labelResize->setText(QString("Размер окна: %1 x %2").arg(width()).arg(height()));
    QMainWindow::resizeEvent(event);
}
