#include <QCoreApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>

void printPortsInfo();

int main(int argc, char *argv[])
  {
      QCoreApplication coreApplication(argc, argv);
      const int argumentCount = QCoreApplication::arguments().size();
           const QStringList argumentList = QCoreApplication::arguments();

           QTextStream standardOutput(stdout);

           if (argumentCount == 1) {
               standardOutput << QObject::tr("Usage: %1 <serialportname> [baudrate]")
                                 .arg(argumentList.first()) << endl;
               return 1;
           }

           QSerialPort serialPort;
           const QString serialPortName = argumentList.at(1);
           serialPort.setPortName(serialPortName);

           const int serialPortBaudRate = (argumentCount > 2)
                   ? argumentList.at(2).toInt() : QSerialPort::Baud9600;
           serialPort.setBaudRate(serialPortBaudRate);

           if (!serialPort.open(QIODevice::ReadOnly)) {
               standardOutput << QObject::tr("Failed to open port %1, error: %2")
                                 .arg(serialPortName).arg(serialPort.error()) << endl;
               return 1;
           }

           QByteArray readData = serialPort.readAll();
           while (serialPort.waitForReadyRead(5000))
               readData.append(serialPort.readAll());

           if (serialPort.error() == QSerialPort::ReadError) {
               standardOutput << QObject::tr("Failed to read from port %1, error: %2")
                                 .arg(serialPortName).arg(serialPort.errorString()) << endl;
               return 1;
           } else if (serialPort.error() == QSerialPort::TimeoutError && readData.isEmpty()) {
               standardOutput << QObject::tr("No data was currently available"
                                             " for reading from port %1")
                                 .arg(serialPortName) << endl;
               return 0;
           }

           standardOutput << QObject::tr("Data successfully received from port %1")
                             .arg(serialPortName) << endl;
           standardOutput << readData << endl;


      return 0;
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
