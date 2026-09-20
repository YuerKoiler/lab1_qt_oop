#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QMouseEvent;
class QKeyEvent;
class QResizeEvent;
class QPaintEvent;
class QEvent;
class QTimer;
class QWidget;
class QPushButton;
class QDate;
class QPoint;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

    // ловит мышь и клавиатуру по окну
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    // меню
    void onActionAboutTriggered();
    void onActionExitTriggered();
    void onActionClearTriggered();
    void onActionResetTriggered();

    // основные сигналы, слоты
    void onMainButtonClicked();
    void onSliderValueChanged(int value);
    void onSpinBoxValueChanged(int value);
    void onCheckToggled(int state);
    void onRadioToggled(bool checked);
    void onComboChanged(int index);
    void onListChanged(int row);
    void onDialChanged(int value);
    void onScrollChanged(int value);
    void onTabChanged(int index);

    // один обработчик на несколько кнопок
    void onColorButtonClicked();

    // программный вызов обработчика и события
    void onCallHandlerClicked();
    void onCallEventClicked();

    // таймер
    void onTimerToggleClicked();
    void onTimerTick();

    // динамические кнопки
    void onDynamicButtonClicked();
    void onDeleteDynamicClicked();

    // выбор даты
    void onDateChanged(const QDate &date);

private:
    void enableMouseTrackingRecursive(QWidget *widget);
    void logDebug(const QString &text);
    void handleKeyEvent(QKeyEvent *event, bool pressed);
    void createDynamicButton(const QPoint &pos);

    Ui::MainWindow *ui;
    QTimer *timer;
    int tickCount;
    bool timerRunning;
    int paintCount;
    QList<QPushButton*> dynamicButtons;
};

#endif // MAINWINDOW_H
