#ifndef LOGGER_H
#define LOGGER_H
#include <QObject>
#include <QFile>
#include <QSerialPort>



class Logger: public QObject
{
  Q_OBJECT

  QFile* datafile = nullptr;
  QFile* logfile = nullptr;
  QSerialPort* serport = nullptr;
public:
  explicit Logger() = default;
   ~Logger() = default;
  void setFiles(QFile* pdatafile, QFile* plogfile, QSerialPort* pserport);
  void close();
public slots:
  void read();
};

#endif // LOGGER_H
