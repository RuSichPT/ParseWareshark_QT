#include "inc/mywidget.h"
#include "ui_mywidget.h"
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QDebug>

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

    // Запускаем диалоговое окно выбора файла
    QString nameFile = QFileDialog::getOpenFileName(this, "Открыть файл","", "*pcap");

    // Устанавиливаем выбранный файл
    m_file->setFileName(nameFile);

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
            ui->m_table->setRowCount(missedPkt.PktNumbers.size()+ui->m_table->rowCount());
            ui->m_table->setItem(row,RX_COLUMN,rxItem);

            foreach (const auto &PktNumbers, missedPkt.PktNumbers)
            {
                uint16_t addrfrom = PktNumbers.addrFrom;
                uint8_t PktNumber = PktNumbers.PktNumber;
                int numbFrameBeac = PktNumbers.numbFrameBeac;

                // Создаем ячейки для таблицы
                QTableWidgetItem *txItem = new QTableWidgetItem(QString::number(addrfrom));
                QTableWidgetItem *ptkNumbItem = new QTableWidgetItem(QString::number(PktNumber));
                QTableWidgetItem *beacNumbItem = new QTableWidgetItem(QString::number(numbFrameBeac));

                ptkNumbItem->setTextAlignment(Qt::AlignCenter);
                txItem->setTextAlignment(Qt::AlignCenter);
                beacNumbItem->setTextAlignment(Qt::AlignCenter);

                // Вставляем ячейки в таблицу
                ui->m_table->setItem(row, TX_COLUMN, txItem);
                ui->m_table->setItem(row, PKT_NUMB_COLUMN, ptkNumbItem);
                ui->m_table->setItem(row, BEAC_NUMB_COLUMN, beacNumbItem);

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
