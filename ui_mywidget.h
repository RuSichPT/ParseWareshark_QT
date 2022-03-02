/********************************************************************************
** Form generated from reading UI file 'mywidget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MYWIDGET_H
#define UI_MYWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MyWidget
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QTableWidget *m_table;
    QProgressBar *m_pBar;
    QPushButton *m_button;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MyWidget)
    {
        if (MyWidget->objectName().isEmpty())
            MyWidget->setObjectName(QString::fromUtf8("MyWidget"));
        MyWidget->resize(500, 400);
        centralwidget = new QWidget(MyWidget);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        m_table = new QTableWidget(centralwidget);
        if (m_table->columnCount() < 5)
            m_table->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        m_table->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        m_table->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        m_table->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        m_table->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        m_table->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        m_table->setObjectName(QString::fromUtf8("m_table"));
        m_table->setEnabled(true);
        m_table->horizontalHeader()->setVisible(true);
        m_table->horizontalHeader()->setDefaultSectionSize(90);
        m_table->verticalHeader()->setVisible(true);
        m_table->verticalHeader()->setDefaultSectionSize(23);

        gridLayout->addWidget(m_table, 0, 0, 1, 2);

        m_pBar = new QProgressBar(centralwidget);
        m_pBar->setObjectName(QString::fromUtf8("m_pBar"));
        m_pBar->setValue(0);

        gridLayout->addWidget(m_pBar, 1, 0, 1, 1);

        m_button = new QPushButton(centralwidget);
        m_button->setObjectName(QString::fromUtf8("m_button"));

        gridLayout->addWidget(m_button, 1, 1, 1, 1);

        MyWidget->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MyWidget);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 500, 21));
        MyWidget->setMenuBar(menubar);
        statusbar = new QStatusBar(MyWidget);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MyWidget->setStatusBar(statusbar);

        retranslateUi(MyWidget);

        QMetaObject::connectSlotsByName(MyWidget);
    } // setupUi

    void retranslateUi(QMainWindow *MyWidget)
    {
        MyWidget->setWindowTitle(QCoreApplication::translate("MyWidget", "ParseWireshark", nullptr));
        QTableWidgetItem *___qtablewidgetitem = m_table->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MyWidget", "Rx", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = m_table->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MyWidget", "Tx", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = m_table->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MyWidget", "numPkt", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = m_table->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MyWidget", "numFramePkt", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = m_table->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MyWidget", "numFrameBeac", nullptr));
        m_button->setText(QCoreApplication::translate("MyWidget", "\320\236\321\202\320\272\321\200\321\213\321\202\321\214", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MyWidget: public Ui_MyWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MYWIDGET_H
