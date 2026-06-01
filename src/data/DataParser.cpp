#include "data/DataParser.h"

#include <QtGlobal>

namespace {

bool inRange(double value, double min, double max)
{
    return value >= min && value <= max;
}

bool isValidStatusCode(const QString& statusCode)
{
    return statusCode == "OK"
        || statusCode == "WARN"
        || statusCode == "ALARM"
        || statusCode == "ERROR";
}

QString rangeError(const QString& field, double value, double min, double max)
{
    return QString("%1 out of range: %2 not in [%3, %4]")
        .arg(field)
        .arg(value)
        .arg(min)
        .arg(max);
}

} // namespace

DeviceData DataParser::parse(const QByteArray& frame)
{
    return parseFrame(frame).data;
}

ParseResult DataParser::parseFrame(const QByteArray& frame)
{
    ParseResult result;
    const QByteArray trimmedFrame = frame.trimmed();
    if (trimmedFrame.isEmpty()) {
        result.error = "empty frame";
        return result;
    }

    QList<QByteArray> parts = trimmedFrame.split(',');
    if (parts.size() != 6) {
        result.error = QString("invalid field count: %1").arg(parts.size());
        return result;
    }

    bool okTemp = false;
    bool okHumidity = false;
    bool okPressure = false;
    bool okCo2 = false;
    bool okDoor = false;

    const double temp = parts[0].trimmed().toDouble(&okTemp);
    const double humidity = parts[1].trimmed().toDouble(&okHumidity);
    const double pressure = parts[2].trimmed().toDouble(&okPressure);
    const double co2 = parts[3].trimmed().toDouble(&okCo2);
    const int door = parts[4].trimmed().toInt(&okDoor);
    const QString statusCode = QString::fromUtf8(parts[5].trimmed()).toUpper();

    if (!okTemp || !okHumidity || !okPressure || !okCo2 || !okDoor) {
        result.error = "numeric conversion failed";
        return result;
    }
    if (!inRange(temp, DeviceDataLimits::TEMP_MIN, DeviceDataLimits::TEMP_MAX)) {
        result.error = rangeError("temperature", temp,
                                  DeviceDataLimits::TEMP_MIN,
                                  DeviceDataLimits::TEMP_MAX);
        return result;
    }
    if (!inRange(humidity, DeviceDataLimits::HUM_MIN, DeviceDataLimits::HUM_MAX)) {
        result.error = rangeError("humidity", humidity,
                                  DeviceDataLimits::HUM_MIN,
                                  DeviceDataLimits::HUM_MAX);
        return result;
    }
    if (!inRange(pressure, DeviceDataLimits::PRESS_MIN, DeviceDataLimits::PRESS_MAX)) {
        result.error = rangeError("pressure", pressure,
                                  DeviceDataLimits::PRESS_MIN,
                                  DeviceDataLimits::PRESS_MAX);
        return result;
    }
    if (!inRange(co2, DeviceDataLimits::CO2_MIN, DeviceDataLimits::CO2_MAX)) {
        result.error = rangeError("co2", co2,
                                  DeviceDataLimits::CO2_MIN,
                                  DeviceDataLimits::CO2_MAX);
        return result;
    }
    if (door != 0 && door != 1) {
        result.error = QString("invalid door state: %1").arg(door);
        return result;
    }
    if (!isValidStatusCode(statusCode)) {
        result.error = "invalid status code: " + statusCode;
        return result;
    }

    result.data.timestamp = QDateTime::currentDateTime();
    result.data.temperature = temp;
    result.data.humidity = humidity;
    result.data.pressure = pressure;
    result.data.co2 = co2;
    result.data.doorOpen = (door == 1);
    result.data.statusCode = statusCode;
    result.data.isValid = true;
    return result;
}
