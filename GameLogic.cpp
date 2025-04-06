#include "GameLogic.h"
#include <QRandomGenerator>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>
#include <QDir>

GameLogic::GameLogic() {
    loadWordList();
    startNewGame();
}

void GameLogic::startNewGame(){
    if(!wordList.isEmpty()){
        int index = QRandomGenerator::global()->bounded(wordList.size());
        targetWord = wordList[index];
    }

    attemptsLeft = 5;
    correctPositions = QVector<QChar>(4, QChar());
    correctLetter.clear();
}

void GameLogic::loadWordList(){
    QFile file(":/words.txt");

    if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream in(&file);

        while(!in.atEnd()){
            QString line = in.readLine().trimmed().toLower();

            if(line.length() == 4){wordList.append(line);}
        }
        file.close();
    }
    if(wordList.isEmpty()){
        wordList << "барс" << "лось" << "рука" << "нога";
    }
}

GameLogic::GuessResult GameLogic::submitGuess(const QString &guess){
    GuessResult result;
    result.remainingAttemps = --attemptsLeft;
    result.isCorrect = (guess == targetWord);
    result.gameOver = false;

    QSet<QChar> currentGuessedLetter;

    for(int i = 0; i < 4; i++){
        QChar gChar = guess[i];
        QChar tChar = targetWord[i];

        if(gChar == tChar){
            correctPositions[i] = gChar;
            correctLetter.insert(gChar);
        } else if (targetWord.contains(gChar) && !correctLetter.contains(gChar)){
            currentGuessedLetter.insert(gChar);
        }
    }

    result.mask = generateMask();
    QStringList guessedLetterList;

    for(const QChar &C : currentGuessedLetter){
        guessedLetterList << C;
    }

    result.guessedLetters = guessedLetterList.join(", ");

    if(result.guessedLetters.isEmpty()){
        result.guessedLetters = "-";
    }

    if(result.isCorrect){
        int score = calculateScore();

        result.message = QString("Вы отгадали слово и заработали %1 баллов. Продолжите?").arg(score);
        result.gameOver = true;
    } else if(attemptsLeft <= 0){
        result.message = QString("Попытки закончились. Счет %1").arg(calculateScore());
        result.gameOver = true;
    } else {
        result.message = QString("Отгаданные буквы: %1\nСлово: %2").arg(result.guessedLetters).arg(result.mask);
    }

    return result;
}

int GameLogic::calculateScore() const {
    switch(6 - attemptsLeft){
    case 1: return 5;
    case 2: return 4;
    case 3: return 3;
    case 4: return 2;
    case 5: return 1;
    case 6: return 0;
    default: return 0;
    }
}

QString GameLogic::getScoresPath() const{
    QString path = "C:/Users/cherk/folderForVSCode/qt/wordle1/scores";
    QDir().mkpath(QFileInfo(path).path());

    qDebug() << "Scores file path:" << path;

    return path;
}

void GameLogic::saveScoreToFile(int score) const{
    if(score <= 0){
        qDebug() << "Zero score - not saving";
        return;
    }

    QString path = getScoresPath();
    QFile file(path);

    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << score << "\n";
        file.close();
        qDebug() << "Successfully saved score:" << score << "to" << path;
    } else {
        qDebug() << "Failed to save score, error:" << file.errorString();
    }
}

QString GameLogic::generateMask() const{
    QString mask;

    for(int i = 0; i < 4; i++){
        if(correctPositions[i].isNull()){mask += "_ ";}
        else{
            mask += correctPositions[i];
            mask += " ";
        }
    }

    return mask.trimmed();
}

QString GameLogic::getTargetWord() const{
    return targetWord;
}
