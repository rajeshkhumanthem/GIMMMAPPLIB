#ifndef GIMMMAPPLIBTEST_H
#define GIMMMAPPLIBTEST_H

#include <QObject>

#include <QtTest/QtTest>

/*You need to include the QTest header and declare the
test functions as private slots so the test framework finds and executes it.
*/
class GimmmAppLibTest: public QObject
{
        Q_OBJECT
  private slots:
    void initTestCase(){}
    void expbackoffTestDefaultConstructor();
    void expbackoffTestConstructorInitialValue();
    void expbackoffTestMaxRetry();
    void cleanupTestCase(){}


};

#endif // GIMMMAPPLIBTEST_H
