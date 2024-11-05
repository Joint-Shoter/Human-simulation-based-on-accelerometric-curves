#ifndef ANIMATIONTHREAD_H
#define ANIMATIONTHREAD_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <QVector3D>
#include <QFile>
#include <QTextStream>
#include <QString>

class AnimationThread : public QObject
{
    Q_OBJECT

public:
    AnimationThread(QObject* parent = nullptr);
    ~AnimationThread();

signals:
    void refreshAnim(QVector<QVector3D> bonePos, QVector<QVector3D> boneRotate);
    void readerError(QString err);

public slots:
    void startReadFile(const QString &filePath, int timeout);
    void stop();

private slots:
    void readNextData();

private:
    QTimer* timer;
    QVector<QVector3D> bonePos;
    QVector<QVector3D> boneRotate;
    QFile dataFile;
    int numBones;
};

#endif // ANIMATIONTHREAD_H
