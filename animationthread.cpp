#include "animationthread.h"
//--------------------------------
//Вычисления выполняемые в другом потоке:
//чтение из выбранного файла данных
//перерасчет для для всех указанных датчиков новых координат
//перемещение костей в соответсвии с новыми координатами
//-----------------------------------
AnimationThread::AnimationThread(QObject* parent)
    : QObject(parent), timer(new QTimer(this)), numBones(0), deltaTime(0.2f)
{
    connect(timer, &QTimer::timeout, this, &AnimationThread::readNextData);
}

AnimationThread::~AnimationThread()
{
    stop();
}

void AnimationThread::setDeltaTime(float delta)
{
    deltaTime = delta;
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
    prevSpeed.clear();

    QString firstLine = dataFile.readLine().trimmed();
    bool ok;
    numBones = firstLine.toInt(&ok);

    if (!ok || numBones <= 0) {
        emit readerError("Invalid number of bones in the file.");
        dataFile.close();
        return;
    }

    bonePos.resize(numBones);
    boneRotate.resize(numBones);
    prevSpeed.resize(numBones, QVector3D(0, 0, 0));

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

    for (int i = 0; i < numBones; ++i) {
        QString line = dataFile.readLine().trimmed();

        if (line.isEmpty()) {
            emit readerError("Empty line for bone " + QString::number(i));
            return;
        }

        QStringList values = line.split(' ', Qt::SkipEmptyParts);

        if (values.size() < 6) {
            emit readerError("Incorrect data format for bone " + QString::number(i));
            return;
        }

        bool ok;
        float ax = values[0].toFloat(&ok);  if (!ok) { emit readerError("Invalid ax"); return; }
        float ay = values[1].toFloat(&ok);  if (!ok) { emit readerError("Invalid ay"); return; }
        float az = values[2].toFloat(&ok);  if (!ok) { emit readerError("Invalid az"); return; }
        float xRot = values[3].toFloat(&ok);  if (!ok) { emit readerError("Invalid xRot"); return; }
        float yRot = values[4].toFloat(&ok);  if (!ok) { emit readerError("Invalid yRot"); return; }
        float zRot = values[5].toFloat(&ok);  if (!ok) { emit readerError("Invalid zRot"); return; }

        QVector3D newSpeed = prevSpeed[i] + QVector3D(ax, ay, az) * deltaTime;

        bonePos[i] = bonePos[i] + newSpeed * deltaTime;
        boneRotate[i] = QVector3D(xRot, yRot, zRot);

        prevSpeed[i] = newSpeed;
    }

    emit refreshAnim(bonePos, boneRotate);

    if (dataFile.atEnd()) {
        stop();
        emit readerError("End of file reached.");
    }
}

