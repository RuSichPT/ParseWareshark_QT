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
        NUM_PKT_COLUMN,
        NUM_FRAME_PKT_COLUMN,
        NUM_FRAME_BEAC_COLUMN,
        COUNT_COLUMNS
    };

public:
    MyWidget(QWidget *parent = nullptr);
    ~MyWidget();

public slots:
    void onClickedButton();
    void onClickedHeader(int logicalIndex);
    void updateBar(uint64_t readBytes);

private:
    Ui::MyWidget    *ui;
    QFile           *m_file;

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private:
    void printMissedPkt(const QVector<FileParser::missedPkt_t> &missedPkts);
    void createMenus();
    void openFile(const QString &fileName);
    void clearTable();
};
#endif // MYWIDGET_H
