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
        QString line = dataFile.readLine().trimmed();  // Читаем строку и удаляем лишние пробелы

        if (line.isEmpty()) {
            emit readerError("Empty line for bone " + QString::number(i));
            return;
        }

        QStringList values = line.split(' ', Qt::SkipEmptyParts);  // Разбиваем строку по пробелам

        if (values.size() < 6) {
            emit readerError("Incorrect data format for bone " + QString::number(i));
            return;
        }

        bool ok;
        float xPos = values[0].toFloat(&ok);  if (!ok) { emit readerError("Invalid xPos"); return; }
        float yPos = values[1].toFloat(&ok);  if (!ok) { emit readerError("Invalid yPos"); return; }
        float zPos = values[2].toFloat(&ok);  if (!ok) { emit readerError("Invalid zPos"); return; }
        float xRot = values[3].toFloat(&ok);  if (!ok) { emit readerError("Invalid xRot"); return; }
        float yRot = values[4].toFloat(&ok);  if (!ok) { emit readerError("Invalid yRot"); return; }
        float zRot = values[5].toFloat(&ok);  if (!ok) { emit readerError("Invalid zRot"); return; }

        bonePos[i] = QVector3D(xPos, yPos, zPos);
        boneRotate[i] = QVector3D(xRot, yRot, zRot);
    }

    emit refreshAnim(bonePos, boneRotate);

    if (dataFile.atEnd()) {
        stop();
        emit readerError("End of file reached.");
    }
}

