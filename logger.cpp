#include "logger.h"
#include <iostream>
#include <QTextStream>
#include <QDateTime>


void printData(const QByteArray& data, QTextStream& out){
  const QString time = QTime::currentTime().toString("hh:mm:ss.zzz");
  QString text_str;
  text_str = time + "\tRead\t" + QString::number(data.size()) +  "\tbyte(s):";
  QString data_str = text_str;
  QString chars{};
  for (auto ch : data){
    data_str+=' ' + QByteArray(1, ch).toHex();
    if (std::isprint(ch)){
      text_str += ch;
      chars += ch;
    }
    else {
      text_str += ".";
    }
  }
  std::cout << data_str.toStdString() << "\n" << text_str.toStdString() << "\n";
  out << data_str << '\t' << chars << '\n';
}

void Logger::setFiles(QFile* pdatafile, QFile* plogfile, QSerialPort* pserport)
  {
    datafile = pdatafile;
    logfile = plogfile;
    serport = pserport;

    if (!serport->open(QIODevice::ReadOnly)) {
      throw "Failed to open serial port";
    }

    if (!datafile->open(QIODevice::WriteOnly) || !logfile->open(QIODevice::WriteOnly)){
      close();
      throw "Opening log file error";
    }
  }
void Logger::close(){
    std::cout << "close\n";

    if ( datafile != nullptr)
      datafile->close();

    if ( datafile != nullptr)
      logfile->close();

    if (datafile != nullptr)
      serport->close();
  }


void Logger::read(){
    QTextStream data_out(datafile);
    QTextStream text_out(logfile);
    QByteArray reception_buffer;
    reception_buffer = serport->readAll();
        while (serport->waitForReadyRead(10)) {
            reception_buffer.append(serport->readAll());
        }
    printData(reception_buffer, text_out);
    data_out << reception_buffer;
    reception_buffer.clear();  
}


