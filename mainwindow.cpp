#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "GameLogic.h"
#include <QListWidget>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <algorithm>

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
    connect(ui->menuGameButton, &QPushButton::clicked, [this](){
        ui->recordsWidget->hide();
        recordsVisible = false;
        setActiveButton(ui->menuGameButton);
    });
    connect(ui->menuRecordsButton, &QPushButton::clicked, this, &MainWindow::toggleRecordsView);
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
        if(result.isCorrect){
            totalScore += gameLogic->calculateScore();
            QMessageBox::StandardButton reply = QMessageBox::question(
                this->window(), "Продолжить?", "Хотите продолжить?", QMessageBox::Yes | QMessageBox::No);

            if(reply == QMessageBox::Yes){
                gameLogic->startNewGame();
                ui->logTextEdit->append("\nНовое слово загадано: ****");
                clearGameUI();
            } else{
                gameActive = false;
                ui->submitGuessButton->setEnabled(false);
                ui->guessInput->setEnabled(false);
                gameLogic->saveScoreToFile(totalScore);
                updateRecords();
            }
        }else{
            gameActive = false;
            ui->submitGuessButton->setEnabled(false);
            ui->guessInput->setEnabled(false);
            gameLogic->saveScoreToFile(totalScore);
            updateRecords();
        }
    }
    ui->maskLabel->setText(result.mask);
}

void MainWindow::toggleRecordsView(){
    if(!recordsVisible){
        updateRecords();
        ui->recordsWidget->show();
        recordsVisible = true;
        setActiveButton(ui->menuRecordsButton);
    } else {
        ui->recordsWidget->hide();
        recordsVisible = false;
        setActiveButton(ui->menuGameButton);
    }
}

void MainWindow::updateRecords(){
    loadScores();
}

void MainWindow::loadScores(){
    ui->recordList->clear();

    QString scoresPath = QCoreApplication::applicationDirPath() + "/scores.txt";

    QFile file(scoresPath);

    if(!file.exists()){
        file.open(QIODevice::WriteOnly);
        file.close();
        return;
    }
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&file);
        QList<int> scores;

        while(!in.atEnd()){
            QString line = in.readLine();
            bool ok;
            int score = line.toInt(&ok);

            if(ok) scores.append(score);
        }

        file.close();

        std::sort(scores.begin(), scores.end(), std::greater<int>());

        for(int score : scores){
            ui->recordList->addItem(QString::number(score));
        }
    }
}

void MainWindow::clearGameUI(){
    ui->maskLabel->setText("_ _ _ _");
    ui->logTextEdit->clear();
    ui->guessInput->clear();
}
