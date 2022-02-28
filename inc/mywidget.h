#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QMainWindow>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFile>
#include <QProgressBar>
#include <QTableWidget>
#include "fileparser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MyWidget; }
QT_END_NAMESPACE

class MyWidget : public QMainWindow
{
    Q_OBJECT

private:
    enum TableColumns
    {
        RX_COLUMN,
        TX_COLUMN,
        PKT_NUMB_COLUMN,
        BEAC_NUMB_COLUMN,
        COUNT_COLUMNS
    };

public:
    MyWidget(QWidget *parent = nullptr);
    ~MyWidget();

public slots:
    void openFile();
    void updateBar(uint64_t readBytes);

private:
    Ui::MyWidget    *ui;
    QFile           *m_file;

private:
    void printMissedPkt(QVector<FileParser::missedPkt_t> &missedPkts);
    void createMenus();
    void clearTable();
};
#endif // MYWIDGET_H
