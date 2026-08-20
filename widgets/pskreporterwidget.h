#ifndef PSKREPORTERWIDGET_H
#define PSKREPORTERWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QStringList>
#include "logbook/logbook.h"

class Configuration;
class MultiSettings;
class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class LogBook;
class QTimer;

namespace Ui {
class PSKReporterWidget;
}

class PSKReporterWidget : public QWidget
{
    Q_OBJECT

public:
    QTableWidget pskTable;
    explicit PSKReporterWidget(QWidget *parent = nullptr, Configuration *cfg = nullptr, LogBook *log = nullptr, MultiSettings *settings = nullptr);
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
    MultiSettings * m_settings;
    QTimer * m_refreshTimer;
    bool m_geometryRestored = false;

private slots:
    void responseHandler(QNetworkReply * reply);
    void on_pskTable_cellDoubleClicked(int row, int column);

protected:
    void showEvent(QShowEvent * event) override;
    void closeEvent(QCloseEvent * event) override;
    void resizeEvent(QResizeEvent * event) override;


};

#endif // PSKREPORTERWIDGET_H
