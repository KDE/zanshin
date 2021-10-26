/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */


#ifndef ZANSHIN_TESTLIB_FAKEJOB_H
#define ZANSHIN_TESTLIB_FAKEJOB_H

#include <KJob>

class QTimer;

class FakeJob : public KJob
{
    Q_OBJECT
public:
    static const int DURATION = 50;
    explicit FakeJob(QObject *parent = nullptr);

    void setExpectedError(int errorCode, const QString &errorText = QString());

    void start() override;

private slots:
    virtual void onTimeout();

public:
    bool isDone() const;
    int expectedError() const;
    QString expectedErrorText() const;

private:
    QTimer *m_timer;
    bool m_done;
    bool m_launched;
    int m_errorCode;
    QString m_errorText;
};

#endif
