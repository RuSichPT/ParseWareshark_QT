#include "inc/mywidget.h"
#include "ui_mywidget.h"
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QDebug>
#include <QSettings>

MyWidget::MyWidget(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MyWidget)
{
    ui->setupUi(this);

    // Файл
    m_file = new QFile(this);
    connect(ui->m_button, &QPushButton::clicked, this, &MyWidget::openFile);

    // Иконка
    QIcon icon = QIcon(":/images/icon.jpg");
    setWindowIcon(icon);

    // Создаем меню
    createMenus();
}

MyWidget::~MyWidget()
{
    delete ui;
}

void MyWidget::openFile()
{
    // Очищаем таблицу
    clearTable();

    // Создаем запись в реестре для сохранения пути
    QSettings settings("ParseWireshark","path");
    QString dir = settings.value("path").toString();

    // Запускаем диалоговое окно выбора файла
    QString nameFile = QFileDialog::getOpenFileName(this, "Открыть файл", dir, "*pcap");

    // Устанавиливаем выбранный файл
    m_file->setFileName(nameFile);

    // Устанавливаем значение в реестре
    settings.setValue("path", nameFile);

    // Открываем файл
    if (!m_file->open(QIODevice::ReadOnly))
    {
        QMessageBox msgBox;
        msgBox.setText("Cannot open file for reading");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }

    // Чтение из файла
    FileParser parser(this);
    parser.readFile(*m_file);
    m_file->close();

    // Рисуем пропущенные пакеты
    printMissedPkt(parser.getMissedPkts());
}

void MyWidget::updateBar(uint64_t readBytes)
{
    int progress;
    progress = (readBytes*100)/(m_file->size());
    ui->m_pBar->setValue(progress);
}

void MyWidget::printMissedPkt(QVector<FileParser::missedPkt_t> &missedPkts)
{
    int row{0};

    if (!missedPkts.empty())
    {
        foreach (const auto &missedPkt, missedPkts)
        {
            // Tx ячейка
            QTableWidgetItem *rxItem = new QTableWidgetItem(QString::number(missedPkt.addr));
            rxItem->setTextAlignment(Qt::AlignCenter);
            // Кол-во строк
            ui->m_table->setRowCount(missedPkt.pktNumbers.size()+ui->m_table->rowCount());
            ui->m_table->setItem(row, RX_COLUMN, rxItem);

            foreach (const auto &pktNumbers, missedPkt.pktNumbers)
            {
                uint16_t addrfrom = pktNumbers.addrFrom;
                uint8_t pktNumber = pktNumbers.pktNumber;
                int pktFrameNumber = pktNumbers.pktFrameNumber;
                int beacFrameNumber = pktNumbers.beacFrameNumber;

                // Создаем ячейки для таблицы
                QTableWidgetItem *txItem = new QTableWidgetItem(QString::number(addrfrom));
                QTableWidgetItem *ptkNumbItem = new QTableWidgetItem(QString::number(pktNumber));
                QTableWidgetItem *pktFrameItem = new QTableWidgetItem(QString::number(pktFrameNumber));
                QTableWidgetItem *beacNumbItem = new QTableWidgetItem(QString::number(beacFrameNumber));

                ptkNumbItem->setTextAlignment(Qt::AlignCenter);
                txItem->setTextAlignment(Qt::AlignCenter);
                pktFrameItem->setTextAlignment(Qt::AlignCenter);
                beacNumbItem->setTextAlignment(Qt::AlignCenter);

                // Вставляем ячейки в таблицу
                ui->m_table->setItem(row, TX_COLUMN, txItem);
                ui->m_table->setItem(row, NUM_PKT_COLUMN, ptkNumbItem);
                ui->m_table->setItem(row, NUM_FRAME_PKT_COLUMN, pktFrameItem);
                ui->m_table->setItem(row, NUM_FRAME_BEAC_COLUMN, beacNumbItem);

                row++;
            }
        }
    }
}

void MyWidget::clearTable()
{
    ui->m_table->clearContents();
    ui->m_table->setRowCount(0);
}

void MyWidget::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu("&Файл");

    QAction *openAct = new QAction("&Открыть", this);
    fileMenu->addAction(openAct);
    connect(openAct, &QAction::triggered, this, &MyWidget::openFile);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction("&Выход");
    connect(exitAct, &QAction::triggered, this, &QWidget::close);
}
