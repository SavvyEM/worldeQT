#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QPushButton>
#include "GameLogic.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class GameLogic;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewGameClicked();
    void onSubmitGuessClicked();
    void toggleRecordsView();
    void updateRecords();

private:
    Ui::MainWindow *ui;
    GameLogic* gameLogic;
    int totalScore;
    bool gameActive;
    bool recordsVisible;

    QPushButton* activeMenuButton;

    void setupConnection();
    void clearGameUI();
    void setActiveButton(QPushButton* button);
    void loadScores();
};
#endif // MAINWINDOW_H
