#include "data/DataParser.h"

#include <QtTest/QtTest>

class DataParserTest : public QObject {
    Q_OBJECT

private slots:
    void acceptsValidFrame();
    void rejectsWrongFieldCount();
    void rejectsOutOfRangeValue();
    void rejectsInvalidDoorState();
    void rejectsInvalidStatusCode();
};

void DataParserTest::acceptsValidFrame()
{
    ParseResult result = DataParser::parseFrame("-18.0,85.3,0.101200,856,0,OK");

    QVERIFY2(result.isValid(), qPrintable(result.error));
    QCOMPARE(result.data.temperature, -18.0);
    QCOMPARE(result.data.humidity, 85.3);
    QCOMPARE(result.data.pressure, 0.1012);
    QCOMPARE(result.data.co2, 856.0);
    QCOMPARE(result.data.doorOpen, false);
    QCOMPARE(result.data.statusCode, QString("OK"));
}

void DataParserTest::rejectsWrongFieldCount()
{
    ParseResult result = DataParser::parseFrame("-18.0,85.3,0.101200");

    QVERIFY(!result.isValid());
    QVERIFY(result.error.contains("field count"));
}

void DataParserTest::rejectsOutOfRangeValue()
{
    ParseResult result = DataParser::parseFrame("-99.0,85.3,0.101200,856,0,OK");

    QVERIFY(!result.isValid());
    QVERIFY(result.error.contains("temperature"));
}

void DataParserTest::rejectsInvalidDoorState()
{
    ParseResult result = DataParser::parseFrame("-18.0,85.3,0.101200,856,3,OK");

    QVERIFY(!result.isValid());
    QVERIFY(result.error.contains("door"));
}

void DataParserTest::rejectsInvalidStatusCode()
{
    ParseResult result = DataParser::parseFrame("-18.0,85.3,0.101200,856,0,BAD");

    QVERIFY(!result.isValid());
    QVERIFY(result.error.contains("status"));
}

QTEST_MAIN(DataParserTest)

#include "tst_dataparser.moc"
