#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBrush>

#include "model.h"
#include "view.h"

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /** attributes **/
    Model md;
    int oldLine0;
    int oldLine1;
    bool isPaused;
    int oldLogLine;

    /** methods **/
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void visualizeTarjan(QString filename);
    void updateCodeColor(int linePos, QString logSearchid);

private slots:
    void on_startButton_clicked();
    void on_pauseButton_clicked();
    void on_resetButton_clicked();

    void on_replayButton_clicked();

    void on_horizontalSlider_actionTriggered(int action);

private:
    Ui::MainWindow *ui;

};

#endif // MAINWINDOW_H
