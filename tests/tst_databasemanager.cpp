#include "data/DatabaseManager.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest/QtTest>

class DatabaseManagerTest : public QObject {
    Q_OBJECT

private slots:
    void exportCsvWritesUtf8Bom();
};

void DatabaseManagerTest::exportCsvWritesUtf8Bom()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    DatabaseManager manager;
    QVERIFY(manager.initialize(dir.filePath("test.db")));

    DeviceData data;
    data.timestamp = QDateTime::currentDateTime();
    data.temperature = -18.5;
    data.humidity = 82.0;
    data.pressure = 0.1013;
    data.co2 = 650.0;
    data.doorOpen = false;
    data.statusCode = "OK";
    data.isValid = true;

    manager.saveData(data);
    manager.flush();

    const QString csvPath = dir.filePath("export.csv");
    QVERIFY(manager.exportToCsv(csvPath,
                                data.timestamp.addSecs(-1),
                                data.timestamp.addSecs(1)));

    QFile csv(csvPath);
    QVERIFY(csv.open(QIODevice::ReadOnly));
    const QByteArray content = csv.readAll();
    QVERIFY(content.size() > 3);
    QCOMPARE(content.left(3), QByteArray("\xEF\xBB\xBF", 3));
    QVERIFY(QString::fromUtf8(content).contains(QStringLiteral("时间,温度")));
}

QTEST_MAIN(DatabaseManagerTest)

#include "tst_databasemanager.moc"
