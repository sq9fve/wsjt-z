#ifndef PSKREPORTERWIDGET_H
#define PSKREPORTERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QStringList>
#include "logbook/logbook.h"

class Configuration;
class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class LogBook;

namespace Ui {
class PSKReporterWidget;
}

class PSKReporterWidget : public QWidget
{
    Q_OBJECT

public:
    QTableWidget pskTable;
    explicit PSKReporterWidget(QWidget *parent = nullptr, Configuration *cfg = nullptr, LogBook *log = nullptr);
    ~PSKReporterWidget();
    void setFont (QFont f);

    Q_SIGNAL void clicked(QString callsing, QString band);
    Q_SIGNAL void reportsUpdated(QStringList const& receiver_report_records);

public slots:
    void refresh(bool init = false);
    void updateTable(QString data);
    void scrollToBottom();

private:
    Ui::PSKReporterWidget *ui;
    QNetworkAccessManager * networkManager;
    Configuration * m_config;
    LogBook * m_logBook;


private slots:
    void responseHandler(QNetworkReply * reply);
    void on_pskTable_cellDoubleClicked(int row, int column);


};

#endif // PSKREPORTERWIDGET_H
