/*
 * SPDX-FileCopyrightText: 2014 Kevin Ottens <ervin@kde.org>
 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/


#include <testlib/qtest_zanshin.h>

#include "domain/queryresult.h"

using namespace Domain;

class Base
{
public:
    typedef QSharedPointer<Base> Ptr;

    virtual ~Base() {}
    virtual QString whoAmI() { return QStringLiteral("I'm Base"); }
};

class Derived : public Base
{
public:
    typedef QSharedPointer<Derived> Ptr;

    QString whoAmI() override { return QStringLiteral("I'm Derived"); }
};

class QueryResultTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void shouldBeCreatedEmpty()
    {
        QueryResultProvider<QString>::Ptr provider(new QueryResultProvider<QString>);
        QVERIFY(provider->data().isEmpty());

        QueryResult<QString>::Ptr result = QueryResult<QString>::create(provider);
        QVERIFY(result->data().isEmpty());
    }

    void shouldHaveSameContent()
    {
        QueryResultProvider<QString>::Ptr provider(new QueryResultProvider<QString>);
        QueryResult<QString>::Ptr result = QueryResult<QString>::create(provider);

        provider->append(QStringLiteral("Bar"));

        QVERIFY(!provider->data().isEmpty());
        QVERIFY(!result->data().isEmpty());

        QCOMPARE(provider->data().size(), 1);
        QCOMPARE(result->data().size(), 1);
        QCOMPARE(provider->data().first(), QStringLiteral("Bar"));
        QCOMPARE(result->data().first(), QStringLiteral("Bar"));

        provider->prepend(QStringLiteral("Foo"));
        *provider << QStringLiteral("Baz");

        QCOMPARE(provider->data().size(), 3);
        QCOMPARE(result->data().size(), 3);
        QCOMPARE(provider->data().first(), QStringLiteral("Foo"));
        QCOMPARE(result->data().first(), QStringLiteral("Foo"));
        QCOMPARE(provider->data().at(1), QStringLiteral("Bar"));
        QCOMPARE(result->data().at(1), QStringLiteral("Bar"));
        QCOMPARE(provider->data().last(), QStringLiteral("Baz"));
        QCOMPARE(result->data().last(), QStringLiteral("Baz"));
    }

    void shouldCreateResultFromAnotherResultOfSameType()
    {
        auto provider = QueryResultProvider<QString>::Ptr::create();
        auto result = QueryResult<QString>::create(provider);

        provider->append(QStringLiteral("Foo"));
        provider->append(QStringLiteral("Bar"));
        provider->append(QStringLiteral("Baz"));

        QVERIFY(!provider->data().isEmpty());
        QVERIFY(!result->data().isEmpty());
        QCOMPARE(result->data(), provider->data());

        auto otherResult = QueryResult<QString>::copy(result);
        QCOMPARE(otherResult->data(), result->data());
    }

    void shouldCreateResultFromAnotherResultOfCompatibleType()
    {
        auto provider = QueryResultProvider<Derived::Ptr>::Ptr::create();
        auto result = QueryResult<Derived::Ptr>::create(provider);

        provider->append(Derived::Ptr::create());

        QList<Base::Ptr> baseList;
        baseList << provider->data().first();

        QVERIFY(!provider->data().isEmpty());
        QVERIFY(!result->data().isEmpty());
        QCOMPARE(result->data(), provider->data());

        auto otherResult = QueryResult<Derived::Ptr, Base::Ptr>::copy(result);
        QCOMPARE(otherResult->data(), baseList);
    }

    void shouldProperlyCopyNullPointers()
    {
        QueryResult<QString>::Ptr result;
        QVERIFY(QueryResult<QString>::copy(result).isNull());
    }

    void shouldResultsKeepProviderAlive()
    {
        QueryResultProvider<QString>::WeakPtr provider;
        QVERIFY(provider.isNull());

        {
            QueryResult<QString>::Ptr result1;

            {
                QueryResultProvider<QString>::Ptr strongProvider(new QueryResultProvider<QString>);
                provider = strongProvider;
                QVERIFY(!provider.isNull());
                result1 = QueryResult<QString>::create(provider.toStrongRef());
            }
            QVERIFY(!provider.isNull());

            {
                QueryResult<QString>::Ptr result2 = QueryResult<QString>::create(provider.toStrongRef());
                Q_UNUSED(result2);
                QVERIFY(!provider.isNull());
            }
            QVERIFY(!provider.isNull());
        }
        QVERIFY(provider.isNull());
    }

    void shouldNotifyInserts()
    {
        QList<QString> preInserts, postInserts;
        QList<int> preInsertsPos, postInsertsPos;

        QueryResultProvider<QString>::Ptr provider(new QueryResultProvider<QString>);
        QueryResult<QString>::Ptr result = QueryResult<QString>::create(provider);

        result->addPreInsertHandler(
            [&](const QString &value, int pos)
            {
                preInserts << value;
                preInsertsPos << pos;
            }
        );

        result->addPostInsertHandler(
            [&](const QString &value, int pos)
            {
                postInserts << value;
                postInsertsPos << pos;
            }
        );

        provider->append(QStringLiteral("Bar"));
        provider->prepend(QStringLiteral("Foo"));
        *provider << QStringLiteral("Baz");
        provider->insert(1, QStringLiteral("Bazz"));

        const QList<QString> expectedInserts = {"Bar", "Foo", "Baz", "Bazz"};
        const QList<int> expectedInsertsPos = {0, 0, 2, 1};
        QCOMPARE(preInserts, expectedInserts);
        QCOMPARE(preInsertsPos, expectedInsertsPos);
        QCOMPARE(postInserts, expectedInserts);
        QCOMPARE(postInsertsPos, expectedInsertsPos);
    }

    void shouldNotifyInsertsForCompatibleTypes()
    {
        QList<Base::Ptr> preInserts, postInserts;
        QList<int> preInsertsPos, postInsertsPos;

        auto provider = QueryResultProvider<Derived::Ptr>::Ptr::create();
        auto derivedResult = QueryResult<Derived::Ptr>::create(provider);
        auto baseResult = QueryResult<Derived::Ptr, Base::Ptr>::copy(derivedResult);

        baseResult->addPreInsertHandler(
            [&](const Base::Ptr &value, int pos)
            {
                preInserts << value;
                preInsertsPos << pos;
            }
        );

        baseResult->addPostInsertHandler(
            [&](const Base::Ptr &value, int pos)
            {
                postInserts << value;
                postInsertsPos << pos;
            }
        );

        provider->append(Derived::Ptr::create());
        provider->append(Derived::Ptr::create());

        const QList<Base::Ptr> expectedInserts = { provider->data().first(), provider->data().last() };
        const QList<int> expectedInsertsPos = {0, 1};
        QCOMPARE(preInserts, expectedInserts);
        QCOMPARE(preInsertsPos, expectedInsertsPos);
        QCOMPARE(postInserts, expectedInserts);
        QCOMPARE(postInsertsPos, expectedInsertsPos);
    }

    void shouldNotifyRemoves()
    {
        QList<QString> preRemoves, postRemoves;
        QList<int> preRemovesPos, postRemovesPos;

        QueryResultProvider<QString>::Ptr provider(new QueryResultProvider<QString>);
        *provider << QStringLiteral("Foo") << QStringLiteral("Bar") << QStringLiteral("Baz") << QStringLiteral("Bazz");

        QueryResult<QString>::Ptr result = QueryResult<QString>::create(provider);

        result->addPreRemoveHandler(
            [&](const QString &value, int pos)
            {
                preRemoves << value;
                preRemovesPos << pos;
            }
        );

        result->addPostRemoveHandler(
            [&](const QString &value, int pos)
            {
                postRemoves << value;
                postRemovesPos << pos;
            }
        );

        provider->removeAt(1);
        provider->removeFirst();
        provider->removeLast();

        const QList<QString> expectedRemoves = {"Bar", "Foo", "Bazz"};
        const QList<int> expectedRemovesPos = {1, 0, 1};
        QCOMPARE(preRemoves, expectedRemoves);
        QCOMPARE(preRemovesPos, expectedRemovesPos);
        QCOMPARE(postRemoves, expectedRemoves);
        QCOMPARE(postRemovesPos, expectedRemovesPos);
    }

    void shouldNotifyTakes()
    {
        QList<QString> preRemoves, postRemoves, taken;
        QList<int> preRemovesPos, postRemovesPos;

        QueryResultProvider<QString>::Ptr provider(new QueryResultProvider<QString>);
        *provider << QStringLiteral("Foo") << QStringLiteral("Bar") << QStringLiteral("Baz") << QStringLiteral("Bazz");

        QueryResult<QString>::Ptr result = QueryResult<QString>::create(provider);

        result->addPreRemoveHandler(
            [&](const QString &value, int pos)
            {
                preRemoves << value;
                preRemovesPos << pos;
            }
        );

        result->addPostRemoveHandler(
            [&](const QString &value, int pos)
            {
                postRemoves << value;
                postRemovesPos << pos;
            }
        );

        taken << provider->takeAt(1);
        taken << provider->takeFirst();
        taken << provider->takeLast();

        const QList<QString> expectedRemoves = {"Bar", "Foo", "Bazz"};
        const QList<int> expectedRemovesPos = {1, 0, 1};
        QCOMPARE(preRemoves, expectedRemoves);
        QCOMPARE(preRemovesPos, expectedRemovesPos);
        QCOMPARE(postRemoves, expectedRemoves);
        QCOMPARE(postRemovesPos, expectedRemovesPos);
        QCOMPARE(taken, expectedRemoves);
    }

    void shouldNotifyReplaces()
    {
        QList<QString> preReplaces, postReplaces;
        QList<int> preReplacesPos, postReplacesPos;

        QueryResultProvider<QString>::Ptr provider(new QueryResultProvider<QString>);
        *provider << QStringLiteral("Foo") << QStringLiteral("Foo") << QStringLiteral("Foo") << QStringLiteral("Foo");

        QueryResult<QString>::Ptr result = QueryResult<QString>::create(provider);

        result->addPreReplaceHandler(
            [&](const QString &value, int pos)
            {
                preReplaces << value;
                preReplacesPos << pos;
            }
        );

        result->addPostReplaceHandler(
            [&](const QString &value, int pos)
            {
                postReplaces << value;
                postReplacesPos << pos;
            }
        );

        provider->replace(1, QStringLiteral("Bar"));
        provider->replace(2, QStringLiteral("Baz"));

        const QList<QString> expectedPreReplaces = {"Foo", "Foo"};
        const QList<QString> expectedPostReplaces = {"Bar", "Baz"};
        const QList<int> expectedReplacesPos = {1, 2};
        QCOMPARE(preReplaces, expectedPreReplaces);
        QCOMPARE(preReplacesPos, expectedReplacesPos);
        QCOMPARE(postReplaces, expectedPostReplaces);
        QCOMPARE(postReplacesPos, expectedReplacesPos);
    }
};

ZANSHIN_TEST_MAIN(QueryResultTest)

#include "queryresulttest.moc"
