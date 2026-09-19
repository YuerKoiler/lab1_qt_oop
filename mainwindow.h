#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QMouseEvent;
class QKeyEvent;
class QResizeEvent;
class QEvent;
class QTimer;
class QWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    // ловит перемещение мыши по всему окну, включая дочерние виджеты
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    // меню
    void onActionAboutTriggered();
    void onActionExitTriggered();
    void onActionClearTriggered();
    void onActionResetTriggered();

    // основные сигналы/слоты (п.5)
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

    // один обработчик на несколько кнопок (п.6)
    void onColorButtonClicked();

    // программный вызов обработчика и события (п.7)
    void onCallHandlerClicked();
    void onCallEventClicked();

    // таймер (п.8) — со своим отдельным прогрессбаром
    void onTimerToggleClicked();
    void onTimerTick();

private:
    void enableMouseTrackingRecursive(QWidget *widget);
    void logDebug(const QString &text);
    void handleKeyEvent(QKeyEvent *event, bool pressed);

    Ui::MainWindow *ui;
    QTimer *timer;
    int tickCount;
    bool timerRunning;
};

#endif // MAINWINDOW_H
