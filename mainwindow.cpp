#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "GameLogic.h"
#include <QListWidget>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , gameLogic(new GameLogic())
    , totalScore(0)
    , gameActive(false)
    , recordsVisible(false)
    , activeMenuButton(nullptr)
{
    ui->setupUi(this);
    ui->recordsWidget->setVisible(false);

    if(!ui->recordsWidget->layout()){
        QVBoxLayout *layout = new QVBoxLayout(ui->recordsWidget);
        layout->addWidget(ui->recordList);
        ui->recordsWidget->setLayout(layout);
    }

    ui->recordList->setStyleSheet("QListWidget { font-size: 14px; }");
    ui->recordsWidget->setAttribute(Qt::WA_ShowWithoutActivating);
    ui->recordsWidget->setStyleSheet("background: white; border: 1px solid #ccc;");

    ui->recordsWidget->hide();

    qDebug() << "Records widget visible: " << ui->recordsWidget->isVisible();

    setupConnection();
    setActiveButton(ui->menuGameButton);
    clearGameUI();
}

MainWindow::~MainWindow(){
    delete ui;
    delete gameLogic;
}

void MainWindow::setupConnection() {
    connect(ui->newGameButton, &QPushButton::clicked, this, [this](){
        this->onNewGameClicked();
    });
    connect(ui->submitGuessButton, &QPushButton::clicked, this, &MainWindow::onSubmitGuessClicked);
    connect(ui->menuGameButton, &QPushButton::clicked, this, &MainWindow::switchToGamePage);
    connect(ui->menuRecordsButton, &QPushButton::clicked, this, &MainWindow::switchToRecordsPage);
}

void MainWindow::setActiveButton(QPushButton* button){
    if(activeMenuButton){
        activeMenuButton->setStyleSheet("");

        activeMenuButton = button;
        activeMenuButton->setStyleSheet(
            "QPushButton{"
            "   background-color: #4CAF50;"
            "   color: white;"
            "   border: none;"
            "   padding: 5px 10px;"
            "   text-align: center;"
            "   text-decoration: none;"
            "   font-size: 14px;"
            "   margin: 4px 2px;"
            "   border-radius: 4px;"
            "}"
            );
    }
}

void MainWindow::onNewGameClicked(){
    if(gameActive){gameLogic->saveScoreToFile(totalScore);}

    totalScore = 0;
    gameActive = true;
    gameLogic->startNewGame();
    clearGameUI();
    ui->logTextEdit->append("Новая игра. Загаданное слово: ****");
    ui->submitGuessButton->setEnabled(true);
    ui->guessInput->setEnabled(true);
    switchToGamePage();
}

void MainWindow::onSubmitGuessClicked(){
    QString guess = ui->guessInput->text().trimmed().toLower();

    if(guess.length() != 4){
        QMessageBox::warning(this->window(), "Ошибка", "Введите 4-буквенное слово!");
        return;
    }

    ui->guessInput->clear();

    auto result = gameLogic->submitGuess(guess);
    ui->logTextEdit->append("Вы ввели: " + guess);
    ui->logTextEdit->append(result.message);

    if(result.gameOver){
        gameLogic->saveScoreToFile(totalScore);
        updateRecords();
    }
    ui->maskLabel->setText(result.mask);

    if (result.isCorrect) {
        int score = gameLogic->calculateScore();
        totalScore += score;
        gameLogic->saveScoreToFile(score);

        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Продолжить?",
            QString("Вы отгадали слово и заработали %1 баллов. Желаете продолжить?").arg(score),
            QMessageBox::Yes | QMessageBox::No
            );

        if (reply == QMessageBox::Yes) {
            gameLogic->startNewGame();
            clearGameUI();
        } else {
            gameActive = false;
            ui->submitGuessButton->setEnabled(false);
            ui->guessInput->setEnabled(false);
        }
    }
}

void MainWindow::toggleRecordsView(){
    if(ui->recordsWidget->isHidden()) {
        updateRecords();

        QPoint pos = ui->menuRecordsButton->mapToParent(QPoint(0, 0));
        ui->recordsWidget->move(pos.x(), pos.y() + ui->menuRecordsButton->height() + 5);

        ui->recordsWidget->show();
        ui->recordsWidget->raise();
        qDebug() << "recordsWidget shown at:" << ui->recordsWidget->pos();
    } else {
        ui->recordsWidget->hide();
    }
}

void MainWindow::updateRecords(){
    loadScores();
}

void MainWindow::loadScores(){
    ui->recordList->clear();

    QString path = gameLogic->getScoresPath();
    QFile file(path);

    if (!file.exists()) {
        qDebug() << "Scores file does not exist, creating empty one";
        ui->recordList->addItem("Рекордов пока нет!");

        if (file.open(QIODevice::WriteOnly)) {
            file.close();
        }
        return;
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QList<int> scores;

        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            bool ok;
            int score = line.toInt(&ok);

            if (ok) {
                scores.append(score);
            }
        }
        file.close();

        if (scores.isEmpty()) {
            ui->recordList->addItem("Рекордов пока нет!");
        } else {
            std::sort(scores.begin(), scores.end());
            for (int i = 0; i < scores.size(); ++i) {
                QListWidgetItem* item = new QListWidgetItem(QString("%1. %2 баллов").arg(i+1).arg(scores[i]));
                item->setTextAlignment(Qt::AlignCenter);
                ui->recordList->addItem(item);
            }
        }
    } else {
        qDebug() << "Failed to open scores file, error:" << file.errorString();
        ui->recordList->addItem("Ошибка загрузки рекордов");
    }

    ui->recordsWidget->show();
    recordsVisible = true;
}

void MainWindow::clearGameUI(){
    ui->maskLabel->setText("_ _ _ _");
    ui->logTextEdit->clear();
    ui->guessInput->clear();
}

void MainWindow::switchToGamePage(){
    ui->stackedWidget->setCurrentIndex(0);
    ui->menuGameButton->setChecked(true);
    ui->menuRecordsButton->setChecked(false);
    ui->recordsWidget->hide();
    recordsVisible = false;
    setActiveButton(ui->menuGameButton);
}

void MainWindow::switchToRecordsPage(){
    updateRecords();
    ui->stackedWidget->setCurrentIndex(1);
    ui->menuRecordsButton->setChecked(true);
    ui->menuGameButton->setChecked(false);
    ui->submitGuessButton->setEnabled(true);
    ui->guessInput->setEnabled(true);
    ui->recordsWidget->setStyleSheet("background-color: black;");
}
