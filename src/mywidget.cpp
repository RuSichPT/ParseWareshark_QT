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

    // Таблицы
    connect(ui->m_table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MyWidget::onClickedTableHeader);
    connect(ui->m_table2->horizontalHeader(), &QHeaderView::sectionClicked, this, &MyWidget::onClickedTable2Header);
    // Запрещаем изменения в таблице
    ui->m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->m_table2->setEditTriggers(QAbstractItemView::NoEditTriggers);

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

    // Рисуем пакеты
    printMissedPkts(parser.getMissedPkts());
    printRepeatedPkts(parser.getRepeatedPkts());
}

void MyWidget::onClickedButton()
{
    // Создаем запись в реестре для сохранения пути
    QSettings settings("ParseWireshark","path");
    QString dir = settings.value("path").toString();

    // Запускаем диалоговое окно выбора файла
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", dir, "*.pcap");

    // Устанавливаем значение в реестре
    settings.setValue("path", fileName);

    openFile(fileName);
}

void MyWidget::onClickedTableHeader(int logicalIndex)
{
    static int orders[(int)TableColumns_t::COUNT_COLUMNS]{0};
    orders[logicalIndex] = !orders[logicalIndex];
    ui->m_table->sortByColumn(logicalIndex, (Qt::SortOrder) orders[logicalIndex]);
    qDebug() << logicalIndex;
}

void MyWidget::onClickedTable2Header(int logicalIndex)
{
    static int orders[(int)Table2Columns_t::COUNT_COLUMNS]{0};
    orders[logicalIndex] = !orders[logicalIndex];
    ui->m_table2->sortByColumn(logicalIndex, (Qt::SortOrder) orders[logicalIndex]);
    qDebug() << logicalIndex;
}

void MyWidget::updateBar(uint64_t readBytes)
{
    int progress;
    progress = (readBytes*100)/(m_file->size());
    ui->m_pBar->setValue(progress);
}

void MyWidget::printMissedPkts(const QVector<FileParser::MissedPkt_t> &missedPkts)
{
    if (!missedPkts.empty())
    {
        int row{0};

        // Кол-во строк
        ui->m_table->setRowCount(missedPkts.size());

        foreach (const auto &missedPkt, missedPkts)
        {
            // Создаем ячейки для таблицы
            QTableWidgetItem *rxItem = new QTableWidgetItem;
            QTableWidgetItem *txItem = new QTableWidgetItem;
            QTableWidgetItem *ptkNumbItem = new QTableWidgetItem;
            QTableWidgetItem *pktFrameItem = new QTableWidgetItem;
            QTableWidgetItem *beacNumbItem = new QTableWidgetItem;

            rxItem->setData(Qt::DisplayRole, missedPkt.destAddr);
            txItem->setData(Qt::DisplayRole, missedPkt.srcAddr);
            ptkNumbItem->setData(Qt::DisplayRole, missedPkt.pktNumber);
            pktFrameItem->setData(Qt::DisplayRole, missedPkt.pktFrameNumber);
            beacNumbItem->setData(Qt::DisplayRole, missedPkt.beacFrameNumber);

            rxItem->setTextAlignment(Qt::AlignCenter);
            txItem->setTextAlignment(Qt::AlignCenter);
            ptkNumbItem->setTextAlignment(Qt::AlignCenter);
            pktFrameItem->setTextAlignment(Qt::AlignCenter);
            beacNumbItem->setTextAlignment(Qt::AlignCenter);

            // Вставляем ячейки в таблицу
            ui->m_table->setItem(row, (int)TableColumns_t::RX_COLUMN, rxItem);
            ui->m_table->setItem(row, (int)TableColumns_t::TX_COLUMN, txItem);
            ui->m_table->setItem(row, (int)TableColumns_t::NUM_PKT_COLUMN, ptkNumbItem);
            ui->m_table->setItem(row, (int)TableColumns_t::NUM_FRAME_PKT_COLUMN, pktFrameItem);
            ui->m_table->setItem(row, (int)TableColumns_t::NUM_FRAME_BEAC_COLUMN, beacNumbItem);

            row++;
        }
    }
}

void MyWidget::printRepeatedPkts(const QVector<FileParser::RepeatedPkt_t> &repeatedPkts)
{
    if (!repeatedPkts.empty())
    {
        int row{0};

        // Кол-во строк
        ui->m_table2->setRowCount(repeatedPkts.size());

        for (const auto &repeatedPkt: repeatedPkts)
        {
            // Создаем ячейки для таблицы
            QTableWidgetItem *txItem = new QTableWidgetItem;
            QTableWidgetItem *ptkNumbItem = new QTableWidgetItem;
            QTableWidgetItem *pktFrameItem = new QTableWidgetItem;

            txItem->setData(Qt::DisplayRole, repeatedPkt.srcAddr);
            ptkNumbItem->setData(Qt::DisplayRole, repeatedPkt.pktNumber);
            pktFrameItem->setData(Qt::DisplayRole, repeatedPkt.pktFrameNumber);

            txItem->setTextAlignment(Qt::AlignCenter);
            ptkNumbItem->setTextAlignment(Qt::AlignCenter);
            pktFrameItem->setTextAlignment(Qt::AlignCenter);

            // Вставляем ячейки в таблицу
            ui->m_table2->setItem(row, (int)Table2Columns_t::TX_COLUMN, txItem);
            ui->m_table2->setItem(row, (int)Table2Columns_t::NUM_PKT_COLUMN, ptkNumbItem);
            ui->m_table2->setItem(row, (int)Table2Columns_t::NUM_FRAME_PKT_COLUMN, pktFrameItem);

            row++;
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
