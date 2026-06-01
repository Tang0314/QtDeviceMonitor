#include "alarm/AlarmChecker.h"

#include <QtTest/QtTest>

class AlarmCheckerTest : public QObject {
    Q_OBJECT

private slots:
    void emitsOnceUntilRecovered();
    void recoversAfterValueReturnsToNormal();
    void disabledChannelDoesNotEmit();
    void invalidConfigFallsBackToDefaults();
};

static DeviceData validData(double temperature)
{
    DeviceData data;
    data.timestamp = QDateTime::currentDateTime();
    data.temperature = temperature;
    data.humidity = 80.0;
    data.pressure = 0.1013;
    data.co2 = 500.0;
    data.doorOpen = false;
    data.statusCode = "OK";
    data.isValid = true;
    return data;
}

void AlarmCheckerTest::emitsOnceUntilRecovered()
{
    AlarmChecker checker;
    QSignalSpy triggeredSpy(&checker, &AlarmChecker::alarmTriggered);
    QSignalSpy clearedSpy(&checker, &AlarmChecker::alarmCleared);

    checker.checkData(validData(-14.0));
    checker.checkData(validData(-13.5));

    QCOMPARE(triggeredSpy.count(), 1);
    QCOMPARE(clearedSpy.count(), 0);
}

void AlarmCheckerTest::recoversAfterValueReturnsToNormal()
{
    AlarmChecker checker;
    QSignalSpy triggeredSpy(&checker, &AlarmChecker::alarmTriggered);
    QSignalSpy clearedSpy(&checker, &AlarmChecker::alarmCleared);

    checker.checkData(validData(-14.0));
    checker.checkData(validData(-18.0));

    QCOMPARE(triggeredSpy.count(), 1);
    QCOMPARE(clearedSpy.count(), 1);
    QCOMPARE(clearedSpy.takeFirst().at(0).toString(), QString("温度"));
}

void AlarmCheckerTest::disabledChannelDoesNotEmit()
{
    AlarmChecker checker;
    AlarmConfig tempConfig;
    tempConfig.highLimit = -15.0;
    tempConfig.lowLimit = -23.0;
    tempConfig.enabled = false;
    checker.setTempConfig(tempConfig);

    QSignalSpy triggeredSpy(&checker, &AlarmChecker::alarmTriggered);

    checker.checkData(validData(-14.0));

    QCOMPARE(triggeredSpy.count(), 0);
}

void AlarmCheckerTest::invalidConfigFallsBackToDefaults()
{
    AlarmConfig invalid;
    invalid.highLimit = 1.0;
    invalid.lowLimit = 2.0;
    invalid.enabled = true;

    AlarmConfig normalized = normalizedAlarmConfig(invalid,
                                                   AlarmDefaults::TEMP_HIGH,
                                                   AlarmDefaults::TEMP_LOW);

    QCOMPARE(normalized.highLimit, AlarmDefaults::TEMP_HIGH);
    QCOMPARE(normalized.lowLimit, AlarmDefaults::TEMP_LOW);
    QCOMPARE(normalized.enabled, true);
}

QTEST_MAIN(AlarmCheckerTest)

#include "tst_alarmchecker.moc"
