#include "inc/mywidget.h"
#include "ui_mywidget.h"
#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QDebug>
#include <QSettings>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QHeaderView>

MyWidget::MyWidget(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MyWidget)
{
    ui->setupUi(this);

    // Файл
    m_file = new QFile(this);
    connect(ui->m_button, &QPushButton::clicked, this, &MyWidget::onClickedButton);

    // Таблица
    connect(ui->m_table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MyWidget::onClickedHeader);

    // Иконка
    QIcon icon = QIcon(":/res/images/icon.ico");
    setWindowIcon(icon);

    // Создаем меню
    createMenus();

    // Drag and Drop
    setAcceptDrops(true);
}

MyWidget::~MyWidget()
{
    delete ui;
}

void MyWidget::dragEnterEvent(QDragEnterEvent *event)
{
    QStringList formats = event->mimeData()->formats();

    for (auto &elem : formats)
    {
        qDebug() << elem;
    }

    QString fileName = event->mimeData()->urls()[0].toLocalFile();

    if (fileName.contains(".pcap"))
    {
        event->acceptProposedAction();
    }
}

void MyWidget::dropEvent(QDropEvent *event)
{
    QString fileName = event->mimeData()->urls()[0].toLocalFile();
    openFile(fileName);

    event->acceptProposedAction(); //сообщаем виджету-источнику, что успешно приняли его содержимое.
}

void MyWidget::openFile(const QString &fileName)
{
    // Очищаем таблицу
    clearTable();

    // Устанавиливаем выбранный файл
    m_file->setFileName(fileName);

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

void MyWidget::onClickedButton()
{
    // Создаем запись в реестре для сохранения пути
    QSettings settings("ParseWireshark","path");
    QString dir = settings.value("path").toString();

    // Запускаем диалоговое окно выбора файла
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", dir, "*pcap");

    // Устанавливаем значение в реестре
    settings.setValue("path", fileName);

    openFile(fileName);
}

void MyWidget::onClickedHeader(int logicalIndex)
{
    static int orders[COUNT_COLUMNS]{0};
    orders[logicalIndex] = !orders[logicalIndex];
    ui->m_table->sortByColumn(logicalIndex, (Qt::SortOrder) orders[logicalIndex]);
    qDebug() << logicalIndex;
}

void MyWidget::updateBar(uint64_t readBytes)
{
    int progress;
    progress = (readBytes*100)/(m_file->size());
    ui->m_pBar->setValue(progress);
}

void MyWidget::printMissedPkt(const QVector<FileParser::missedPkt_t> &missedPkts)
{
    if (!missedPkts.empty())
    {
        int row{0};

        foreach (const auto &missedPkt, missedPkts)
        {
            // Кол-во строк
            ui->m_table->setRowCount(missedPkt.pktNumbers.size()+ui->m_table->rowCount());

            foreach (const auto &pktNumbers, missedPkt.pktNumbers)
            {
                uint16_t addrfrom = pktNumbers.addrFrom;
                uint8_t pktNumber = pktNumbers.pktNumber;
                int pktFrameNumber = pktNumbers.pktFrameNumber;
                int beacFrameNumber = pktNumbers.beacFrameNumber;

                // Создаем ячейки для таблицы
                QTableWidgetItem *rxItem = new QTableWidgetItem(QString::number(missedPkt.addr));
                QTableWidgetItem *txItem = new QTableWidgetItem(QString::number(addrfrom));
                QTableWidgetItem *ptkNumbItem = new QTableWidgetItem(QString::number(pktNumber));
                QTableWidgetItem *pktFrameItem = new QTableWidgetItem(QString::number(pktFrameNumber));
                QTableWidgetItem *beacNumbItem = new QTableWidgetItem(QString::number(beacFrameNumber));

                rxItem->setTextAlignment(Qt::AlignCenter);
                txItem->setTextAlignment(Qt::AlignCenter);
                ptkNumbItem->setTextAlignment(Qt::AlignCenter);
                pktFrameItem->setTextAlignment(Qt::AlignCenter);
                beacNumbItem->setTextAlignment(Qt::AlignCenter);

                // Вставляем ячейки в таблицу
                ui->m_table->setItem(row, RX_COLUMN, rxItem);
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
    connect(openAct, &QAction::triggered, this, &MyWidget::onClickedButton);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction("&Выход");
    connect(exitAct, &QAction::triggered, this, &QWidget::close);
}
