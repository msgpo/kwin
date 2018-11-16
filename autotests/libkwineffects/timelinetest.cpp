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

using namespace std::chrono_literals;

// FIXME: Delete it in the future.
Q_DECLARE_METATYPE(std::chrono::milliseconds)

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
    void testRunning();
    void testStrictRedirectSourceMode_data();
    void testStrictRedirectSourceMode();
    void testRelaxedRedirectSourceMode_data();
    void testRelaxedRedirectSourceMode();
    void testStrictRedirectTargetMode_data();
    void testStrictRedirectTargetMode();
    void testRelaxedRedirectTargetMode_data();
    void testRelaxedRedirectTargetMode();
};

void TimeLineTest::testUpdateForward()
{
    KWin::TimeLine timeLine(1000ms, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    // 0/1000
    QCOMPARE(timeLine.value(), 0.0);
    QVERIFY(!timeLine.done());

    // 100/1000
    timeLine.update(100ms);
    QCOMPARE(timeLine.value(), 0.1);
    QVERIFY(!timeLine.done());

    // 400/1000
    timeLine.update(300ms);
    QCOMPARE(timeLine.value(), 0.4);
    QVERIFY(!timeLine.done());

    // 900/1000
    timeLine.update(500ms);
    QCOMPARE(timeLine.value(), 0.9);
    QVERIFY(!timeLine.done());

    // 1000/1000
    timeLine.update(3000ms);
    QCOMPARE(timeLine.value(), 1.0);
    QVERIFY(timeLine.done());
}

void TimeLineTest::testUpdateBackward()
{
    KWin::TimeLine timeLine(1000ms, KWin::TimeLine::Backward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    // 0/1000
    QCOMPARE(timeLine.value(), 1.0);
    QVERIFY(!timeLine.done());

    // 100/1000
    timeLine.update(100ms);
    QCOMPARE(timeLine.value(), 0.9);
    QVERIFY(!timeLine.done());

    // 400/1000
    timeLine.update(300ms);
    QCOMPARE(timeLine.value(), 0.6);
    QVERIFY(!timeLine.done());

    // 900/1000
    timeLine.update(500ms);
    QCOMPARE(timeLine.value(), 0.1);
    QVERIFY(!timeLine.done());

    // 1000/1000
    timeLine.update(3000ms);
    QCOMPARE(timeLine.value(), 0.0);
    QVERIFY(timeLine.done());
}

void TimeLineTest::testUpdateFinished()
{
    KWin::TimeLine timeLine(1000ms, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    timeLine.update(1000ms);
    QCOMPARE(timeLine.value(), 1.0);
    QVERIFY(timeLine.done());

    timeLine.update(42ms);
    QCOMPARE(timeLine.value(), 1.0);
    QVERIFY(timeLine.done());
}

void TimeLineTest::testToggleDirection()
{
    KWin::TimeLine timeLine(1000ms, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    QCOMPARE(timeLine.value(), 0.0);
    QVERIFY(!timeLine.done());

    timeLine.update(600ms);
    QCOMPARE(timeLine.value(), 0.6);
    QVERIFY(!timeLine.done());

    timeLine.toggleDirection();
    QCOMPARE(timeLine.value(), 0.6);
    QVERIFY(!timeLine.done());

    timeLine.update(200ms);
    QCOMPARE(timeLine.value(), 0.4);
    QVERIFY(!timeLine.done());

    timeLine.update(3000ms);
    QCOMPARE(timeLine.value(), 0.0);
    QVERIFY(timeLine.done());
}

void TimeLineTest::testReset()
{
    KWin::TimeLine timeLine(1000ms, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    timeLine.update(1000ms);
    QCOMPARE(timeLine.value(), 1.0);
    QVERIFY(timeLine.done());

    timeLine.reset();
    QCOMPARE(timeLine.value(), 0.0);
    QVERIFY(!timeLine.done());
}

void TimeLineTest::testSetElapsed_data()
{
    QTest::addColumn<std::chrono::milliseconds>("duration");
    QTest::addColumn<std::chrono::milliseconds>("elapsed");
    QTest::addColumn<std::chrono::milliseconds>("expectedElapsed");
    QTest::addColumn<bool>("expectedDone");
    QTest::addColumn<bool>("initiallyDone");

    QTest::newRow("Less than duration, not finished")    << 1000ms << 300ms  << 300ms  << false << false;
    QTest::newRow("Less than duration, finished")        << 1000ms << 300ms  << 300ms  << false << true;
    QTest::newRow("Greater than duration, not finished") << 1000ms << 3000ms << 1000ms << true  << false;
    QTest::newRow("Greater than duration, finished")     << 1000ms << 3000ms << 1000ms << true  << true;
    QTest::newRow("Equal to duration, not finished")     << 1000ms << 1000ms << 1000ms << true  << false;
    QTest::newRow("Equal to duration, finished")         << 1000ms << 1000ms << 1000ms << true  << true;
}

void TimeLineTest::testSetElapsed()
{
    QFETCH(std::chrono::milliseconds, duration);
    QFETCH(std::chrono::milliseconds, elapsed);
    QFETCH(std::chrono::milliseconds, expectedElapsed);
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
    KWin::TimeLine timeLine(1000ms, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    QCOMPARE(timeLine.duration(), 1000ms);

    timeLine.setDuration(3000ms);
    QCOMPARE(timeLine.duration(), 3000ms);
}

void TimeLineTest::testSetDurationRetargeting()
{
    KWin::TimeLine timeLine(1000ms, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    timeLine.update(500ms);
    QCOMPARE(timeLine.value(), 0.5);
    QVERIFY(!timeLine.done());

    timeLine.setDuration(3000ms);
    QCOMPARE(timeLine.value(), 0.5);
    QVERIFY(!timeLine.done());
}

void TimeLineTest::testSetDurationRetargetingSmallDuration()
{
    KWin::TimeLine timeLine(1000ms, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    timeLine.update(999ms);
    QCOMPARE(timeLine.value(), 0.999);
    QVERIFY(!timeLine.done());

    timeLine.setDuration(3ms);
    QCOMPARE(timeLine.value(), 1.0);
    QVERIFY(timeLine.done());
}

void TimeLineTest::testRunning()
{
    KWin::TimeLine timeLine(1000ms, KWin::TimeLine::Forward);
    timeLine.setEasingCurve(QEasingCurve::Linear);

    QVERIFY(!timeLine.running());
    QVERIFY(!timeLine.done());

    timeLine.update(100ms);
    QVERIFY(timeLine.running());
    QVERIFY(!timeLine.done());

    timeLine.update(900ms);
    QVERIFY(!timeLine.running());
    QVERIFY(timeLine.done());
}

void TimeLineTest::testStrictRedirectSourceMode_data()
{
    QTest::addColumn<KWin::TimeLine::Direction>("initialDirection");
    QTest::addColumn<qreal>("initialValue");
    QTest::addColumn<KWin::TimeLine::Direction>("finalDirection");
    QTest::addColumn<qreal>("finalValue");

    QTest::newRow("forward -> backward") << KWin::TimeLine::Forward  << 0.0 << KWin::TimeLine::Backward << 0.0;
    QTest::newRow("backward -> forward") << KWin::TimeLine::Backward << 1.0 << KWin::TimeLine::Forward  << 1.0;
}

void TimeLineTest::testStrictRedirectSourceMode()
{
    QFETCH(KWin::TimeLine::Direction, initialDirection);
    KWin::TimeLine timeLine(1000ms, initialDirection);
    timeLine.setEasingCurve(QEasingCurve::Linear);
    timeLine.setSourceRedirectMode(KWin::TimeLine::RedirectMode::Strict);

    QTEST(timeLine.direction(), "initialDirection");
    QTEST(timeLine.value(), "initialValue");
    QCOMPARE(timeLine.sourceRedirectMode(), KWin::TimeLine::RedirectMode::Strict);
    QVERIFY(!timeLine.running());
    QVERIFY(!timeLine.done());

    QFETCH(KWin::TimeLine::Direction, finalDirection);
    timeLine.setDirection(finalDirection);

    QTEST(timeLine.direction(), "finalDirection");
    QTEST(timeLine.value(), "finalValue");
    QCOMPARE(timeLine.sourceRedirectMode(), KWin::TimeLine::RedirectMode::Strict);
    QVERIFY(!timeLine.running());
    QVERIFY(timeLine.done());
}

void TimeLineTest::testRelaxedRedirectSourceMode_data()
{
    QTest::addColumn<KWin::TimeLine::Direction>("initialDirection");
    QTest::addColumn<qreal>("initialValue");
    QTest::addColumn<KWin::TimeLine::Direction>("finalDirection");
    QTest::addColumn<qreal>("finalValue");

    QTest::newRow("forward -> backward") << KWin::TimeLine::Forward  << 0.0 << KWin::TimeLine::Backward << 1.0;
    QTest::newRow("backward -> forward") << KWin::TimeLine::Backward << 1.0 << KWin::TimeLine::Forward  << 0.0;
}

void TimeLineTest::testRelaxedRedirectSourceMode()
{
    QFETCH(KWin::TimeLine::Direction, initialDirection);
    KWin::TimeLine timeLine(1000ms, initialDirection);
    timeLine.setEasingCurve(QEasingCurve::Linear);
    timeLine.setSourceRedirectMode(KWin::TimeLine::RedirectMode::Relaxed);

    QTEST(timeLine.direction(), "initialDirection");
    QTEST(timeLine.value(), "initialValue");
    QCOMPARE(timeLine.sourceRedirectMode(), KWin::TimeLine::RedirectMode::Relaxed);
    QVERIFY(!timeLine.running());
    QVERIFY(!timeLine.done());

    QFETCH(KWin::TimeLine::Direction, finalDirection);
    timeLine.setDirection(finalDirection);

    QTEST(timeLine.direction(), "finalDirection");
    QTEST(timeLine.value(), "finalValue");
    QCOMPARE(timeLine.sourceRedirectMode(), KWin::TimeLine::RedirectMode::Relaxed);
    QVERIFY(!timeLine.running());
    QVERIFY(!timeLine.done());
}

void TimeLineTest::testStrictRedirectTargetMode_data()
{
    QTest::addColumn<KWin::TimeLine::Direction>("initialDirection");
    QTest::addColumn<qreal>("initialValue");
    QTest::addColumn<KWin::TimeLine::Direction>("finalDirection");
    QTest::addColumn<qreal>("finalValue");

    QTest::newRow("forward -> backward") << KWin::TimeLine::Forward  << 0.0 << KWin::TimeLine::Backward << 1.0;
    QTest::newRow("backward -> forward") << KWin::TimeLine::Backward << 1.0 << KWin::TimeLine::Forward  << 0.0;
}

void TimeLineTest::testStrictRedirectTargetMode()
{
    QFETCH(KWin::TimeLine::Direction, initialDirection);
    KWin::TimeLine timeLine(1000ms, initialDirection);
    timeLine.setEasingCurve(QEasingCurve::Linear);
    timeLine.setTargetRedirectMode(KWin::TimeLine::RedirectMode::Strict);

    QTEST(timeLine.direction(), "initialDirection");
    QTEST(timeLine.value(), "initialValue");
    QCOMPARE(timeLine.targetRedirectMode(), KWin::TimeLine::RedirectMode::Strict);
    QVERIFY(!timeLine.running());
    QVERIFY(!timeLine.done());

    timeLine.update(1000ms);
    QTEST(timeLine.value(), "finalValue");
    QVERIFY(!timeLine.running());
    QVERIFY(timeLine.done());

    QFETCH(KWin::TimeLine::Direction, finalDirection);
    timeLine.setDirection(finalDirection);

    QTEST(timeLine.direction(), "finalDirection");
    QTEST(timeLine.value(), "finalValue");
    QVERIFY(!timeLine.running());
    QVERIFY(timeLine.done());
}

void TimeLineTest::testRelaxedRedirectTargetMode_data()
{
    QTest::addColumn<KWin::TimeLine::Direction>("initialDirection");
    QTest::addColumn<qreal>("initialValue");
    QTest::addColumn<KWin::TimeLine::Direction>("finalDirection");
    QTest::addColumn<qreal>("finalValue");

    QTest::newRow("forward -> backward") << KWin::TimeLine::Forward  << 0.0 << KWin::TimeLine::Backward << 1.0;
    QTest::newRow("backward -> forward") << KWin::TimeLine::Backward << 1.0 << KWin::TimeLine::Forward  << 0.0;
}

void TimeLineTest::testRelaxedRedirectTargetMode()
{
    QFETCH(KWin::TimeLine::Direction, initialDirection);
    KWin::TimeLine timeLine(1000ms, initialDirection);
    timeLine.setEasingCurve(QEasingCurve::Linear);
    timeLine.setTargetRedirectMode(KWin::TimeLine::RedirectMode::Relaxed);

    QTEST(timeLine.direction(), "initialDirection");
    QTEST(timeLine.value(), "initialValue");
    QCOMPARE(timeLine.targetRedirectMode(), KWin::TimeLine::RedirectMode::Relaxed);
    QVERIFY(!timeLine.running());
    QVERIFY(!timeLine.done());

    timeLine.update(1000ms);
    QTEST(timeLine.value(), "finalValue");
    QVERIFY(!timeLine.running());
    QVERIFY(timeLine.done());

    QFETCH(KWin::TimeLine::Direction, finalDirection);
    timeLine.setDirection(finalDirection);

    QTEST(timeLine.direction(), "finalDirection");
    QTEST(timeLine.value(), "finalValue");
    QVERIFY(!timeLine.running());
    QVERIFY(!timeLine.done());

    timeLine.update(1000ms);
    QTEST(timeLine.direction(), "finalDirection");
    QTEST(timeLine.value(), "initialValue");
    QVERIFY(!timeLine.running());
    QVERIFY(timeLine.done());
}

QTEST_MAIN(TimeLineTest)

#include "timelinetest.moc"
