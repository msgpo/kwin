/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2018 Vlad Zagorodniy <vladzzag@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include <kwineffects.h>

#include <QtTest>

class TimeLineTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testUpdateForward();
    void testUpdateBackward();
    void testUpdateFinished();
    void testToggleDirection();
    void testReset();
    void testSetElapsed_data();
    void testSetElapsed();
    void testSetDuration();
    void testSetDurationRetargeting();
    void testSetDurationRetargetingSmallDuration();
};

void TimeLineTest::testUpdateForward()
{
    KWin::TimeLine timeLine(1000, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    // 0/1000
    QCOMPARE(timeLine.value(), 0.0);
    QVERIFY(!timeLine.done());

    // 100/1000
    timeLine.update(100);
    QCOMPARE(timeLine.value(), 0.1);
    QVERIFY(!timeLine.done());

    // 400/1000
    timeLine.update(300);
    QCOMPARE(timeLine.value(), 0.4);
    QVERIFY(!timeLine.done());

    // 900/1000
    timeLine.update(500);
    QCOMPARE(timeLine.value(), 0.9);
    QVERIFY(!timeLine.done());

    // 1000/1000
    timeLine.update(3000);
    QCOMPARE(timeLine.value(), 1.0);
    QVERIFY(timeLine.done());
}

void TimeLineTest::testUpdateBackward()
{
    KWin::TimeLine timeLine(1000, KWin::TimeLine::Backward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    // 0/1000
    QCOMPARE(timeLine.value(), 1.0);
    QVERIFY(!timeLine.done());

    // 100/1000
    timeLine.update(100);
    QCOMPARE(timeLine.value(), 0.9);
    QVERIFY(!timeLine.done());

    // 400/1000
    timeLine.update(300);
    QCOMPARE(timeLine.value(), 0.6);
    QVERIFY(!timeLine.done());

    // 900/1000
    timeLine.update(500);
    QCOMPARE(timeLine.value(), 0.1);
    QVERIFY(!timeLine.done());

    // 1000/1000
    timeLine.update(3000);
    QCOMPARE(timeLine.value(), 0.0);
    QVERIFY(timeLine.done());
}

void TimeLineTest::testUpdateFinished()
{
    KWin::TimeLine timeLine(1000, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    timeLine.update(1000);
    QCOMPARE(timeLine.value(), 1.0);
    QVERIFY(timeLine.done());

    timeLine.update(42);
    QCOMPARE(timeLine.value(), 1.0);
    QVERIFY(timeLine.done());
}

void TimeLineTest::testToggleDirection()
{
    KWin::TimeLine timeLine(1000, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    QCOMPARE(timeLine.value(), 0.0);
    QVERIFY(!timeLine.done());

    timeLine.update(600);
    QCOMPARE(timeLine.value(), 0.6);
    QVERIFY(!timeLine.done());

    timeLine.toggleDirection();
    QCOMPARE(timeLine.value(), 0.6);
    QVERIFY(!timeLine.done());

    timeLine.update(200);
    QCOMPARE(timeLine.value(), 0.4);
    QVERIFY(!timeLine.done());

    timeLine.update(3000);
    QCOMPARE(timeLine.value(), 0.0);
    QVERIFY(timeLine.done());
}

void TimeLineTest::testReset()
{
    KWin::TimeLine timeLine(1000, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    timeLine.update(1000);
    QCOMPARE(timeLine.value(), 1.0);
    QVERIFY(timeLine.done());

    timeLine.reset();
    QCOMPARE(timeLine.value(), 0.0);
    QVERIFY(!timeLine.done());
}

void TimeLineTest::testSetElapsed_data()
{
    QTest::addColumn<int>("duration");
    QTest::addColumn<int>("elapsed");
    QTest::addColumn<int>("expectedElapsed");
    QTest::addColumn<bool>("expectedDone");
    QTest::addColumn<bool>("initiallyDone");

    QTest::newRow("Less than duration, not finished")    << 1000 << 300  << 300  << false << false;
    QTest::newRow("Less than duration, finished")        << 1000 << 300  << 300  << false << true;
    QTest::newRow("Greater than duration, not finished") << 1000 << 3000 << 1000 << true  << false;
    QTest::newRow("Greater than duration, finished")     << 1000 << 3000 << 1000 << true  << true;
    QTest::newRow("Equal to duration, not finished")     << 1000 << 1000 << 1000 << true  << false;
    QTest::newRow("Equal to duration, finished")         << 1000 << 1000 << 1000 << true  << true;
}

void TimeLineTest::testSetElapsed()
{
    QFETCH(int, duration);
    QFETCH(int, elapsed);
    QFETCH(int, expectedElapsed);
    QFETCH(bool, expectedDone);
    QFETCH(bool, initiallyDone);

    KWin::TimeLine timeLine(duration, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    if (initiallyDone) {
        timeLine.update(duration);
        QVERIFY(timeLine.done());
    }

    timeLine.setElapsed(elapsed);
    QCOMPARE(timeLine.elapsed(), expectedElapsed);
    QCOMPARE(timeLine.done(), expectedDone);
}

void TimeLineTest::testSetDuration()
{
    KWin::TimeLine timeLine(1000, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    QCOMPARE(timeLine.duration(), 1000);

    timeLine.setDuration(3000);
    QCOMPARE(timeLine.duration(), 3000);
}

void TimeLineTest::testSetDurationRetargeting()
{
    KWin::TimeLine timeLine(1000, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    timeLine.update(500);
    QCOMPARE(timeLine.value(), 0.5);
    QVERIFY(!timeLine.done());

    timeLine.setDuration(3000);
    QCOMPARE(timeLine.value(), 0.5);
    QVERIFY(!timeLine.done());
}

void TimeLineTest::testSetDurationRetargetingSmallDuration()
{
    KWin::TimeLine timeLine(1000, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    timeLine.update(999);
    QCOMPARE(timeLine.value(), 0.999);
    QVERIFY(!timeLine.done());

    timeLine.setDuration(3);
    QCOMPARE(timeLine.value(), 1.0);
    QVERIFY(timeLine.done());
}

QTEST_MAIN(TimeLineTest)

#include "timelinetest.moc"
