#include "data/DataParser.h"

DeviceData DataParser::parse(const QByteArray& frame)
{
    DeviceData data;
    QList<QByteArray> parts = frame.split(',');
    if (parts.size() < 6) return data;

    bool ok1, ok2, ok3, ok4, ok5;
    double temp     = parts[0].trimmed().toDouble(&ok1);
    double humidity = parts[1].trimmed().toDouble(&ok2);
    double pressure = parts[2].trimmed().toDouble(&ok3);
    double co2      = parts[3].trimmed().toDouble(&ok4);
    int    door     = parts[4].trimmed().toInt(&ok5);

    if (!ok1 || !ok2 || !ok3 || !ok4 || !ok5) return data;

    data.timestamp   = QDateTime::currentDateTime();
    data.temperature = temp;
    data.humidity    = humidity;
    data.pressure    = pressure;
    data.co2         = co2;
    data.doorOpen    = (door == 1);
    data.statusCode  = QString::fromUtf8(parts[5].trimmed());
    data.isValid     = true;

    return data;
}
