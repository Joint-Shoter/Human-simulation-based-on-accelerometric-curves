#include "animationthread.h"
//--------------------------------
//Вычисления выполняемые в другом потоке:
//чтение из выбранного файла данных
//перерасчет для для всех указанных датчиков новых координат
//перемещение костей в соответсвии с новыми координатами
//-----------------------------------
AnimationThread::AnimationThread(QObject* parent)
    : QObject(parent), timer(new QTimer(this)), numBones(0)
{
    connect(timer, &QTimer::timeout, this, &AnimationThread::readNextData);
}

AnimationThread::~AnimationThread()
{
    stop();
}

void AnimationThread::startReadFile(const QString &filePath, int timeout)
{
    if (dataFile.isOpen()) {
        dataFile.close();
    }

    dataFile.setFileName(filePath);

    if (!dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit readerError("Failed to open file: " + dataFile.errorString());
        return;
    }

    bonePos.clear();
    boneRotate.clear();

    QTextStream in(&dataFile);
    if (!in.atEnd()) {
        in >> numBones;
        bonePos.resize(numBones);
        boneRotate.resize(numBones);
    }

    timer->start(timeout);
}

void AnimationThread::stop()
{
    timer->stop();
    if (dataFile.isOpen()) {
        dataFile.close();
    }
}

void AnimationThread::readNextData()
{
    if (!dataFile.isOpen()) {
        emit readerError("File not open for reading.");
        return;
    }

    QTextStream in(&dataFile);

    if (!in.atEnd()) {
        for (int i = 0; i < numBones; ++i) {
            float xPos, yPos, zPos;
            float xRot, yRot, zRot;

            if (!(in >> xPos >> yPos >> zPos >> xRot >> yRot >> zRot)) {
                emit readerError("Failed to read data for bone " + QString::number(i));
                return;
            }

            bonePos[i] = QVector3D(xPos, yPos, zPos);
            boneRotate[i] = QVector3D(xRot, yRot, zRot);
        }

        emit refreshAnim(bonePos, boneRotate);
    } else {
        stop();
        emit readerError("End of file reached.");
    }
}

