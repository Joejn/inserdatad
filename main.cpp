#include <QCoreApplication>

#include "insertdata.h"
#include "entertainment.h"

#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    InsertData *insertData = new InsertData;
    Entertainment * entertainment = new Entertainment;

    return a.exec();
}
