#include <QApplication>

#include "CWMain.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  
  CWMain main;

  main.show();

  return app.exec();
}

