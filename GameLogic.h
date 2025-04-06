#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <QString>
#include <QVector>
#include <QSet>
#include <QFile>
#include <QTextStream>
#include <QRandomGenerator>
#include <QDebug>

class GameLogic
{
public:
    GameLogic();
    ~GameLogic() = default;

    void startNewGame();

    struct GuessResult{
        bool isCorrect;
        QString mask;
        QString guessedLetters;
        QString message;
        int remainingAttemps;
        bool gameOver;
    };

    GuessResult submitGuess(const QString &guess);
    int calculateScore() const;
    void saveScoreToFile(int score) const;
    QString getTargetWord() const;
    QString getScoresPath() const;

private:
    QStringList wordList;
    QString targetWord;
    int attemptsLeft;
    QVector<QChar> correctPositions;
    QSet<QChar> correctLetter;
    void loadWordList();
    QString generateMask() const;
};

#endif // GAMELOGIC_H
