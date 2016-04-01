/* This file is part of Zanshin

   Copyright 2016 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
   USA.
*/

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QProcess>
#include <QTcpSocket>

class ProcessKiller
{
public:
    explicit ProcessKiller(QProcess *process)
        : m_process(process)
    {
    }

    ~ProcessKiller()
    {
        m_process->kill();
        m_process->waitForFinished();
    }

private:
    ProcessKiller(const ProcessKiller &other);
    ProcessKiller &operator=(const ProcessKiller &other);

    QProcess *m_process;
};

bool waitForCukeSteps(qint64 timeout)
{
    QElapsedTimer timer;
    timer.start();
    QTcpSocket socket;

    socket.connectToHost(QHostAddress::LocalHost, 3902);
    while (!socket.waitForConnected() && !timer.hasExpired(timeout)) {
        socket.connectToHost(QHostAddress::LocalHost, 3902);
    }

    return socket.state() == QTcpSocket::ConnectedState;
}

int main(int argc, char **argv)
{
    qputenv("ZANSHIN_USER_XMLDATA", USER_XMLDATA);

    QCoreApplication app(argc, argv);

    QDir::setCurrent(QStringLiteral(FEATURES_DIR));

    QProcess cukeSteps;
    cukeSteps.setProcessChannelMode(QProcess::ForwardedChannels);
    cukeSteps.start(QStringLiteral(CUKE_STEPS));

    if (!cukeSteps.waitForStarted()) {
        qWarning() << "Couldn't start the cuke steps server, exiting...";
        return 1;
    }

    ProcessKiller cukeStepsKiller(&cukeSteps);

    if (!waitForCukeSteps(10000)) {
        qWarning() << "The cuke steps server didn't show up as expected, exiting...";
        return 1;
    }

    const QStringList args = app.arguments().contains(QStringLiteral("wip")) ? QStringList()
                                                                             : QStringList({"--tags", "~@wip"});
    return QProcess::execute(QStringLiteral("cucumber"), args);
}
