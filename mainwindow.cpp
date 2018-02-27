///////////////////////////////////////////////////////////////////////////////
// SAM-flash-programmer
//
// Copyright (c) 2018, Wajdi Bazuhair
// All rights reserved.
//
// GPL-3.0 license
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///////////////////////////////////////////////////////////////////////////////

#include "mainwindow.h"
#include "ui_mainwindow.h"



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serialPort = NULL;
    connect(&manager, SIGNAL(finished(QNetworkReply*)),
               SLOT(downloadFinished(QNetworkReply*)));
    ui->listWidget->addItem("Initialization success.");

    if(QSerialPortInfo::availablePorts().size() == 0) {                                   // Check if there are any ports at all; if not, disable controls and return
         //  enableControls(false);
           ui->connectButton->setEnabled(false);
           ui->listWidget->addItem("No ports detected.");
       }
   else
    {
       for(QSerialPortInfo port : QSerialPortInfo::availablePorts()) {                       // List all available serial ports and populate ports combo box
           ui->comboPort->addItem(port.portName());
       }
    }

    fileName = "";

    //connect(ui->Flash_button, SIGNAL (released()), this, SLOT (on_flashButton_clicked()));
}

MainWindow::~MainWindow()
{
    if(serialPort != NULL) delete serialPort;
    delete ui;
}

/******************************************************************************************************************/
/* Open the inside serial port; connect its signals */
/******************************************************************************************************************/
void MainWindow::openPort(QSerialPortInfo portInfo, int baudRate, QSerialPort::DataBits dataBits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits)
{
    serialPort = new QSerialPort(portInfo, 0);                                            // Create a new serial port

    connect(this, SIGNAL(portOpenOK()), this, SLOT(portOpenedSuccess()));                 // Connect port signals to GUI slots
    connect(this, SIGNAL(portOpenFail()), this, SLOT(portOpenedFail()));
    connect(this, SIGNAL(portClosed()), this, SLOT(onPortClosed()));
    connect(this, SIGNAL(newData(QStringList)), this, SLOT(onNewDataArrived(QStringList)));
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
    connect (serialPort, SIGNAL(errorOccurred(QSerialPort::SerialPortError)),this, SLOT(ErrorOccured(QSerialPort::SerialPortError )));

    if(serialPort->open(QIODevice::ReadWrite) ) {
        serialPort->setBaudRate(baudRate);
        serialPort->setParity(parity);
        serialPort->setDataBits(dataBits);
        serialPort->setStopBits(stopBits);
        emit portOpenOK();
    } else {
        emit portOpenedFail();
        qDebug() << serialPort->errorString();
    }

}

void MainWindow::on_flashButton_clicked()
{
    ui->listWidget->addItem("Flashing started...");
    ui->listWidget->addItem("Initiated Reset...");
    ForceReset();
//PerformFlash();

}
void MainWindow::PerformFlash()
{
    ui->comboPort->clear();
   if(QSerialPortInfo::availablePorts().size() == 0) {                                   // Check if there are any ports at all; if not, disable controls and return
        //  enableControls(false);
          ui->connectButton->setEnabled(false);
          ui->listWidget->addItem("No ports detected.");
      }
  else
   {
      for(QSerialPortInfo port : QSerialPortInfo::availablePorts()) {                       // List all available serial ports and populate ports combo box
          ui->comboPort->addItem(port.portName());
      }
   }

    ui->listWidget->addItem("Updating...");
    QString BossacProgram = "bossac.exe";
     ComPort = ui->comboPort->currentText();
    QStringList arguments;
    arguments << "-i" <<"-d" << "--port="+ComPort <<"-e"<<"-w"<<"-v" <<"-b" <<fileName << "-R";
    process = new QProcess(this);
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyRead()));
    connect (process, SIGNAL(readyReadStandardError()), this, SLOT(printError()));
    stream.setDevice(process);
    process->start(BossacProgram, arguments);
}

void MainWindow::onReadyRead()
{
    QByteArray byteArray = process->readAllStandardOutput();
       QStringList strLines = QString(byteArray).split("\n");

       foreach (QString line, strLines){
           ui->listWidget->addItem(line);
       }
}
void MainWindow::printError()
{
    ui->listWidget->addItem("Error occured.");
    QByteArray byteArray = process->readAllStandardError();
       QStringList strLines = QString(byteArray).split("\n");

       foreach (QString line, strLines){
           ui->listWidget->addItem(line);
       }
}

void MainWindow::on_RefreshButton_clicked()
{
     ui->comboPort->clear();
    if(QSerialPortInfo::availablePorts().size() == 0) {                                   // Check if there are any ports at all; if not, disable controls and return
         //  enableControls(false);
           ui->connectButton->setEnabled(false);
           ui->listWidget->addItem("No ports detected.");
       }
   else
    {
       for(QSerialPortInfo port : QSerialPortInfo::availablePorts()) {                       // List all available serial ports and populate ports combo box
           ui->comboPort->addItem(port.portName());
       }
       if(fileName != "")
       ui->connectButton->setEnabled(true);
    }
}
void MainWindow::ForceReset()
{
    QSerialPortInfo portInfo(ui->comboPort->currentText());                           // Temporary object, needed to create QSerialPort
    int baudRate = 1200;//ui->comboBaud->currentText().toInt();                              // Get baud rate from combo box
    int dataBitsIndex =0;// ui->comboData->currentIndex();                                // Get index of data bits combo box
    int parityIndex = 0;//ui->comboParity->currentIndex();                                // Get index of parity combo box
    int stopBitsIndex = 0;//ui->comboStop->currentIndex();                                // Get index of stop bits combo box
    QSerialPort::DataBits dataBits;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBits;

    if(dataBitsIndex == 0) {                                                          // Set data bits according to the selected index
        dataBits = QSerialPort::Data8;
    } else {
        dataBits = QSerialPort::Data7;
    }

    if(parityIndex == 0) {                                                            // Set parity according to the selected index
        parity = QSerialPort::NoParity;
    } else if(parityIndex == 1) {
        parity = QSerialPort::OddParity;
    } else {
        parity = QSerialPort::EvenParity;
    }

    if(stopBitsIndex == 0) {                                                          // Set stop bits according to the selected index
         stopBits = QSerialPort::OneStop;
    } else {
        stopBits = QSerialPort::TwoStop;
    }

    serialPort = new QSerialPort(portInfo, 0);                                        // Use local instance of QSerialPort; does not crash
    openPort(portInfo, baudRate, dataBits, parity, stopBits);

   //  serialPort->close();
}
/******************************************************************************************************************/
/* Connect Button clicked slot; handles connect and disconnect */
/******************************************************************************************************************/
void MainWindow::on_connectButton_clicked()
{

                                                                                 // If application is not connected, connect                                                                                    // Get parameters from controls first
        QSerialPortInfo portInfo(ui->comboPort->currentText());                           // Temporary object, needed to create QSerialPort
        int baudRate = 1200;//ui->comboBaud->currentText().toInt();                              // Get baud rate from combo box
        int dataBitsIndex =0;// ui->comboData->currentIndex();                                // Get index of data bits combo box
        int parityIndex = 0;//ui->comboParity->currentIndex();                                // Get index of parity combo box
        int stopBitsIndex = 0;//ui->comboStop->currentIndex();                                // Get index of stop bits combo box
        QSerialPort::DataBits dataBits;
        QSerialPort::Parity parity;
        QSerialPort::StopBits stopBits;

        if(dataBitsIndex == 0) {                                                          // Set data bits according to the selected index
            dataBits = QSerialPort::Data8;
        } else {
            dataBits = QSerialPort::Data7;
        }

        if(parityIndex == 0) {                                                            // Set parity according to the selected index
            parity = QSerialPort::NoParity;
        } else if(parityIndex == 1) {
            parity = QSerialPort::OddParity;
        } else {
            parity = QSerialPort::EvenParity;
        }

        if(stopBitsIndex == 0) {                                                          // Set stop bits according to the selected index
             stopBits = QSerialPort::OneStop;
        } else {
            stopBits = QSerialPort::TwoStop;
        }

       // serialPort = new QSerialPort(portInfo, 0);
            // Use local instance of QSerialPort; does not crash
        openPort(portInfo, baudRate, dataBits, parity, stopBits);                         // Open serial port and connect its signals


}
/******************************************************************************************************************/


/******************************************************************************************************************/
/* Slot for port opened successfully */
/******************************************************************************************************************/
void MainWindow::portOpenedSuccess()
{
     ui->listWidget->addItem("Forced 1200bps success");
    connected = true;
    QThread::sleep(3);
    //ForceReset();// Set flags
    //wait for reset
if(serialPort!=NULL)
{
     ui->listWidget->addItem("Closing port");
    serialPort->close();
    delete serialPort;                                                                // Delete the pointer
    serialPort = NULL;
}
  //  if(fileName != "")
     //   ui->flashButton->setEnabled(true);
  //   ForceReset();
   //  QThread::sleep(3);
    PerformFlash();
}
/******************************************************************************************************************/


/******************************************************************************************************************/
/* Slot for fail to open the port */
/******************************************************************************************************************/
void MainWindow::portOpenedFail()
{
    ui->listWidget->addItem("Soft Reset failed, cannot open port");
    if(serialPort!=NULL)
    serialPort->close();
    delete serialPort;                                                                // Delete the pointer
    serialPort = NULL;
    PerformFlash();
}
/******************************************************************************************************************/

void MainWindow::ErrorOccured(QSerialPort::SerialPortError error)
{
    static const char * EnumStrings[] = { "NoError"	,
                                         " DeviceNotFoundError",
                                          "PermissionError",
                                          "OpenError",
                                          "NotOpenError"	,
                                          "ParityError",
                                          "FramingError",
                                          "BreakConditionError"	,
                                          "WriteError"	,
                                          "ReadError"	,
                                          "ResourceError"	,
                                          "UnsupportedOperationError",
                                          "TimeoutError"	,
                                          "UnknownError" };
    ui->listWidget->addItem(EnumStrings[error]);
    if(error == 1)
    {
         ui->listWidget->addItem("Reset in progress");
    }
}
/******************************************************************************************************************/
/* Slot for closing the port */
/******************************************************************************************************************/
void MainWindow::onPortClosed()
{

    connected = false;
    disconnect(serialPort, SIGNAL(readyRead()), this, SLOT(readData()));
    disconnect(this, SIGNAL(portOpenOK()), this, SLOT(portOpenedSuccess()));             // Disconnect port signals to GUI slots
    disconnect(this, SIGNAL(portOpenFail()), this, SLOT(portOpenedFail()));
    disconnect(this, SIGNAL(portClosed()), this, SLOT(onPortClosed()));
    disconnect(this, SIGNAL(newData(QStringList)), this, SLOT(onNewDataArrived(QStringList)));
    disconnect (serialPort, SIGNAL(errorOccurred(QSerialPort::SerialPortError)),this, SLOT(ErrorOccured(QSerialPort::SerialPortError)));

    PerformFlash();
}


void MainWindow::doDownload(const QUrl &url)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    currentDownloads.append(reply);
}

QString MainWindow::saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    if (basename.isEmpty())
        basename = "download";

    if (QFile::exists(basename)) {
        // already exists, don't overwrite
        int i = 0;
        basename += '.';
        while (QFile::exists(basename + QString::number(i)))
            ++i;

        basename += QString::number(i);
    }

    return basename;
}

bool MainWindow::saveToDisk(const QString &filename, QIODevice *data)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "Could not open %s for writing: %s\n",
                qPrintable(filename),
                qPrintable(file.errorString()));
        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}

bool MainWindow::isHttpRedirect(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void MainWindow::execute()
{
        QUrl url = QUrl::fromEncoded("http://forum.eboardshop.net/uploads/default/original/1X/5bd684a1da039d5457daf172ce8fc02a2ec8c891.jpg");
        doDownload(url);
}



void MainWindow::downloadFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
    if (reply->error()) {
        ui->listWidget->addItem( qPrintable(reply->errorString()));
    } else {
        if (isHttpRedirect(reply)) {
            ui->listWidget->addItem("Firmware download error, redirect");
        } else {
            QString filename = saveFileName(url);
            if (saveToDisk(filename, reply)) {
                printf("Download of %s succeeded (saved to %s)\n",
                       url.toEncoded().constData(), qPrintable(filename));
            }
        }
    }

    currentDownloads.removeAll(reply);
    reply->deleteLater();

    if (currentDownloads.isEmpty()) {
        // all downloads finished
       ui->listWidget->addItem("Firmware download success");
    }
}

void MainWindow::on_pushButton_3_clicked()
{
   // execute();
    QFileDialog* dialog = new QFileDialog();
    fileName = dialog->getOpenFileName(this,
        tr("Open Bin"), "", tr("Bin Files (*.bin)"));
   //QFileDialog dialog(this);
   QList<QUrl> URLs = dialog->selectedUrls();
   if(fileName != "")
   {
   ui->listWidget->addItem(fileName);
   ui->label_5->setText(fileName);
   if(ui->comboPort->size().rheight() >0)
   ui->connectButton->setEnabled(true);
   }
}
