#include "gimmmapplibtest.h"

#include "exponentialbackoff.h"
#include "gimmmconnection.h"
#include "message.h"

void GimmmAppLibTest::expbackoffTestDefaultConstructor()
{
    ExponentialBackoff ex;
    QVERIFY(ex.getRetry() == 0);
    QVERIFY(ex.getMaxRetry() == -1);
}


void GimmmAppLibTest::expbackoffTestConstructorInitialValue()
{
    ExponentialBackoff ex1(1, 5);
    QVERIFY(ex1.getRetry() == 1);
    QVERIFY(ex1.getMaxRetry() == 6);

    ex1.resetRetry();
    QVERIFY(ex1.getRetry() == 0);
}


void GimmmAppLibTest::expbackoffTestMaxRetry()
{

    ExponentialBackoff ex1(1, 5);
    QVERIFY(ex1.getRetry() == 1);
    QVERIFY(ex1.getMaxRetry() == 6);
    for (int i = 1; i <= 6; i++)
    {
        QVERIFY(i == ex1.getRetry());
        QVERIFY(6 == ex1.getMaxRetry());
        ex1.next();
    }
}

QTEST_MAIN(GimmmAppLibTest)
