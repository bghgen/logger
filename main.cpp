#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTextStream>
#include <iostream>
#include <cctype>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <signal.h>
#include <functional>


void printPortsInfo();
void printData(const QByteArray& data, QTextStream& out);
class Logger{
  QFile* datafile = nullptr;
  QFile* logfile = nullptr;
  QSerialPort* serport = nullptr;

public:
  Logger() = default;
  Logger(QFile* datafile, QFile* logfile, QSerialPort* serport):
    datafile(datafile), logfile(logfile), serport(serport)
  {

    if (!datafile->open(QIODevice::WriteOnly) || !logfile->open(QIODevice::WriteOnly)){
      const QString err = "Opening log file error";
      std::cout << err.toStdString() << std::endl;
      throw err.toStdString().c_str();
    }

    if (!serport->open(QIODevice::ReadOnly)) {
      const QString err = "Failed to open port " + serport->portName();
      close();
      std::cout << err.toStdString() << std::endl;
      throw err.toStdString().c_str();
    }
  }
  void close(){
    std::cout << "close\n";

    if ( datafile != nullptr)
      datafile->close();

    if ( datafile != nullptr)
      logfile->close();

    if (datafile != nullptr)
      serport->close();
  }
};

Logger obj;

void my_handler(int s){
  Q_UNUSED(s)
  obj.close();
  exit(1);
}

int main(int argc, char *argv[]){
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " <serialportname>" << std::endl;
    return 1;
  }

  if (static_cast<std::string>(argv[1]) == "ports"){
    printPortsInfo();
    return 0;
  }

  /////////////////////////////////
  /// создание папки и файлов логов
  /////////////////////////////////
  const QString folder_name = "./com_port_data";
  if (!QDir().exists(folder_name)){
    QDir().mkdir(folder_name);
  }
  const QString current_datetime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
  const QString data_filename = folder_name + '/' + current_datetime + "_recv.dat";
  const QString text_filename = folder_name + '/' + current_datetime + "_recv.txt";
  QFile data_file(data_filename);
  QFile text_file(text_filename);




  QSerialPort serialPort;
  const QString serialPortName(argv[1]);
  serialPort.setPortName(serialPortName);
  const int serialPortBaudRate = QSerialPort::Baud19200;
  serialPort.setBaudRate(serialPortBaudRate);

  try {
  obj = Logger(&data_file, &text_file, &serialPort);
  } catch (const char* str) {
    std::cout << str << std::endl;
    return -1;
  }


  QTextStream data_out(&data_file);
  QTextStream text_out(&text_file);

  /// обработка сигнала Ctrl + C

  struct sigaction sigIntHandler;
  sigIntHandler.sa_handler = my_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, nullptr);
  ////

  QByteArray readData = serialPort.readAll();

  while (serialPort.waitForReadyRead(-1)){ // wait forever
    readData.append(serialPort.readAll());
    printData(readData, text_out);
    data_out << readData;
    readData.clear();
  }

  if (serialPort.error() == QSerialPort::ReadError) {
    std::cout << "Failed to read from port " << serialPortName.toStdString() << ", error: " << serialPort.errorString().toStdString() << std::endl;
    data_file.close();
    text_file.close();
    serialPort.close();
    return 1;
  } else if (serialPort.error() == QSerialPort::TimeoutError && readData.isEmpty()) {
    std::cout << "No data was currently available for reading from port " << serialPortName.toStdString() << std::endl;
    data_file.close();
    text_file.close();
    serialPort.close();
    return 0;
  }

  return 0;
}




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

void printPortsInfo(){
  QTextStream out(stdout);
  const auto serialPortInfos = QSerialPortInfo::availablePorts();

  out << "Total number of ports available: " << serialPortInfos.count() << endl;

  const QString blankString = "N/A";
  QString description;
  QString manufacturer;
  QString serialNumber;

  for (const QSerialPortInfo &serialPortInfo : serialPortInfos) {
      description = serialPortInfo.description();
      manufacturer = serialPortInfo.manufacturer();
      serialNumber = serialPortInfo.serialNumber();
      out << endl
          << "Port: " << serialPortInfo.portName() << endl
          << "Location: " << serialPortInfo.systemLocation() << endl
          << "Description: " << (!description.isEmpty() ? description : blankString) << endl
          << "Manufacturer: " << (!manufacturer.isEmpty() ? manufacturer : blankString) << endl
          << "Serial number: " << (!serialNumber.isEmpty() ? serialNumber : blankString) << endl
          << "Vendor Identifier: " << (serialPortInfo.hasVendorIdentifier()
                                       ? QByteArray::number(serialPortInfo.vendorIdentifier(), 16)
                                       : blankString) << endl
          << "Product Identifier: " << (serialPortInfo.hasProductIdentifier()
                                        ? QByteArray::number(serialPortInfo.productIdentifier(), 16)
                                        : blankString) << endl
          << "Busy: " << (serialPortInfo.isBusy() ? "Yes" : "No") << endl;
  }
}
