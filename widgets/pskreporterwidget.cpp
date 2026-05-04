#include "pskreporterwidget.h"
#include "ui_pskreporterwidget.h"
#include "Configuration.hpp"
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTime>
#include <QApplication>
#include <QtGui>
#include <qobject.h>
#include <QXmlStreamReader>
#include "logbook/AD1CCty.hpp"
#include <QString>

PSKReporterWidget::PSKReporterWidget(QWidget *parent, Configuration * cfg, LogBook * log) :
    QWidget(parent),
    ui(new Ui::PSKReporterWidget)
{
    ui->setupUi(this);
    m_config = cfg;
    m_logBook = log;
    networkManager = new QNetworkAccessManager(this);

    ui->pskTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->pskTable->horizontalHeader()->setStretchLastSection(true);
    ui->pskTable->setFont(m_config->decoded_text_font());
    ui->pskTable->setColumnHidden(7,  true);

    QPushButton *topRightButton = new QPushButton("R", this);
    topRightButton->setGeometry(20, 20, 40, 30);

     // Connect the button to a slot (if needed)
     connect(topRightButton, &QPushButton::clicked, this, &PSKReporterWidget::refresh);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(refresh()));
    timer->start(5 * 60 * 1000);

    refresh(true);
}


PSKReporterWidget::~PSKReporterWidget()
{
    delete ui;
}

void PSKReporterWidget::refresh(bool init) {
    if (!init && !this->isVisible()) return;

    QUrlQuery query;
    query.addQueryItem("flowStartSeconds", "-3600");
    query.addQueryItem("callsign", m_config->my_callsign());
    QUrl url("https://www.pskreporter.info/cgi-bin/pskquery5.pl");
    url.setQuery(query);
    QNetworkRequest networkRequest(url);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(responseHandler(QNetworkReply*)));
    networkManager->get(networkRequest);
}

void PSKReporterWidget::responseHandler(QNetworkReply * reply) {
    disconnect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(responseHandler(QNetworkReply*)));
    if(reply->error() == QNetworkReply::NoError) {
       QString data = (QString)reply->readAll();
       if (data.length()) updateTable(data);

    } else {
        ui->pskTable->setRowCount(0);
        ui->pskTable->insertRow(0);
        ui->pskTable->setItem(0,0, new QTableWidgetItem(reply->errorString()));
    }

    reply->deleteLater();
}

void PSKReporterWidget::updateTable(QString data) {
    ui->pskTable->setRowCount(0);
    ui->pskTable->setSortingEnabled(false);
    QXmlStreamReader reader(data);
    while(!reader.hasError() && !reader.atEnd()) {
        if(reader.readNext() == QXmlStreamReader::StartElement) {
            if ( reader.name() == "receptionReport") {
                QString callsign = reader.attributes().value("receiverCallsign").toString();
                if (callsign == m_config->my_callsign()) continue;
                int r = 0;
                ui->pskTable->insertRow(r);
                QString time = QDateTime::fromTime_t(reader.attributes().value("flowStartSeconds").toUInt()).toUTC().toString("hh:mm:ss");
                ui->pskTable->setItem(r,0, new QTableWidgetItem(time));


                ui->pskTable->setItem(r,1, new QTableWidgetItem(callsign));

                ui->pskTable->setItem(r,2, new QTableWidgetItem(reader.attributes().value("sNR").toString()));
                ui->pskTable->item(r,2)->setTextAlignment(Qt::AlignCenter);

                auto const& looked_up = m_logBook->countries ()->lookup (callsign);
                auto countryName = looked_up.entity_name;
                countryName.replace ("Islands", "Is.");
                countryName.replace ("Island", "Is.");
                countryName.replace ("North ", "N. ");
                countryName.replace ("Northern ", "N. ");
                countryName.replace ("South ", "S. ");
                countryName.replace ("East ", "E. ");
                countryName.replace ("Eastern ", "E. ");
                countryName.replace ("West ", "W. ");
                countryName.replace ("Western ", "W. ");
                countryName.replace ("Central ", "C. ");
                countryName.replace (" and ", " & ");
                countryName.replace ("Republic", "Rep.");
                countryName.replace ("United States", "U.S.A.");
                countryName.replace ("Fed. Rep. of ", "");
                countryName.replace ("French ", "Fr.");
                countryName.replace ("Asiatic", "AS");
                countryName.replace ("European", "EU");
                countryName.replace ("African", "AF");
                ui->pskTable->setItem(r,3, new QTableWidgetItem(countryName));

                float freq = reader.attributes().value("frequency").toFloat();

                freq = freq/1000000;
                QString f = QString::number(freq);
                f = f.left(f.indexOf(".")+4);
                ui->pskTable->setItem(r,4, new QTableWidgetItem(f));

                ui->pskTable->setItem(r,5, new QTableWidgetItem(reader.attributes().value("mode").toString()));

                ui->pskTable->setItem(r,6, new QTableWidgetItem(reader.attributes().value("receiverLocator").toString().left(6)));
                ui->pskTable->item(r,6)->setTextAlignment(Qt::AlignCenter);

                ui->pskTable->setItem(r,7, new QTableWidgetItem(reader.attributes().value("frequency").toString()));

            }

        }
    }

    QTimer::singleShot (0, this, SLOT (scrollToBottom()));

}


void PSKReporterWidget::setFont (QFont f) {
    ui->pskTable->setFont(f);
}

void PSKReporterWidget::scrollToBottom() {
    ui->pskTable->setSortingEnabled(true);
    ui->pskTable->resizeRowsToContents();
    ui->pskTable->scrollToBottom();
}

void PSKReporterWidget::on_pskTable_cellDoubleClicked(int row, int /*column*/)
{
    QString callsign = ui->pskTable->item(row, 1)->text();
    QString band = ui->pskTable->item(row, 7)->text();
    emit clicked(callsign, band);
}
