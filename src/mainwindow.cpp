#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "config.h"

#include <QTableWidgetItem>
#include <QSettings>
#include <QTimer>
#include <QNetworkRequest>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrlQuery>

#include "spdlog/spdlog.h"

#include "spdlog/sinks/qt_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

#include <nlohmann/json.hpp>
#include "json-qt.hpp"

#include <memory>
#include <filesystem>
#include <utility>
#include <fstream>
#include <sstream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow)
{
    QCoreApplication::setApplicationName(PROJECT_NAME);
    QCoreApplication::setApplicationVersion(PROJECT_VER);
    m_ui->setupUi(this);

    auto const log_name{ "log.txt" };

    m_appdir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    std::filesystem::create_directory(m_appdir.toStdString());
    QString logdir = m_appdir + "/log/";
    std::filesystem::create_directory(logdir.toStdString());

    try
    {
        auto file{ std::string(logdir.toStdString() + log_name) };
        auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(file, 1024 * 1024, 5, false);

        m_logger = std::make_shared<spdlog::logger>("box_design", rotating);
        m_logger->flush_on(spdlog::level::debug);
        m_logger->set_level(spdlog::level::debug);
        m_logger->set_pattern("[%D %H:%M:%S] [%L] %v");
        spdlog::register_logger(m_logger);
    }
    catch (std::exception& /*ex*/)
    {
        QMessageBox::warning(this, "Logger Failed", "Logger Failed To Start.");
    }

    setWindowTitle(windowTitle() + " v" + PROJECT_VER);

    m_settings = std::make_unique< QSettings>(m_appdir + "/settings.ini", QSettings::IniFormat);

    m_manager = new QNetworkAccessManager(this);

    connect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::reply_finished);

    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(refresh_statuses()));
    timer->start(60000);

    QTimer::singleShot(1000, this, SLOT(loadList()));
}

MainWindow::~MainWindow()
{
    saveList();
    delete m_ui;
}

void MainWindow::on_pushButtonAdd_clicked()
{
    QString const url = QString("http://%1/api/system/info").arg(m_ui->lineEditIpAddress->text());
    m_manager->get(QNetworkRequest(QUrl(url)));

    QTimer::singleShot(5000, this, SLOT(refresh_statuses()));
}

void MainWindow::reply_finished(QNetworkReply *reply)
{
    QString answer = reply->readAll();
    Read_FPP_JSON(answer);
}

void MainWindow::on_tableWidgetFPP_cellDoubleClicked(int row, int column) 
{
    if (column == 5)
    {
        auto const ip = m_ui->tableWidgetFPP->item(row, 0)->text();
        QDesktopServices::openUrl(QUrl("http://" + ip));
    }

    if (column == 6)
    {
        auto const ip = m_ui->tableWidgetFPP->item(row, 0)->text();
        setTesting(ip, 1);
    }

    if (column == 7)
    {
        auto const ip = m_ui->tableWidgetFPP->item(row, 0)->text();
        setTesting(ip, 0);
    }
}

void MainWindow::Read_FPP_JSON(QString const& json)
{
    if (json.isEmpty()) { return; }
    auto ttt = json.toStdString();
    //QJsonParseError parseError;
    nlohmann::json doc = nlohmann::json::parse(ttt);

   // QJsonDocument doc = QJsonDocument::fromJson(json.toLatin1(), &parseError);
        if (doc.is_array())
        {
            for (auto const& itm : doc)
            {
               // if (itm.is_object()) 
                {
                    add_FPP(FPP_Data(itm));
                }
            }
        }
        else
        {
            FPP_Data item(doc);

            if (!contains_FPP(item)) {
                searchForOthers(item);
                add_FPP(item);
            }
        }

        refreshList();
    
}

void MainWindow::searchForOthers(FPP_Data const& fpp)
{
    QString const url = QString("http://%1/api/fppd/multiSyncSystems").arg(fpp.IP);
    m_manager->get(QNetworkRequest(QUrl(url)));
}

void MainWindow::refreshList() 
{
    m_ui->tableWidgetFPP->clearContents();

    while (m_ui->tableWidgetFPP->rowCount() != 0)
    {
        //if (ui.PartDisplay->cellWidget(0, _settings.TestedColumnNumber()) != NULL) {
           // disconnect(ui.PartDisplay->cellWidget(0, _settings.TestedColumnNumber()), SIGNAL(currentIndexChanged(int)), this, SLOT(comboChanged(int)));
       // }
        m_ui->tableWidgetFPP->removeRow(0);
    }
    

    std::sort(m_FPP_List.begin(), m_FPP_List.end(),
        [](auto const& a, auto const& b) {return a.IP > b.IP; }
    );

    for (auto const& fpp : m_FPP_List)
    {
        int row = m_ui->tableWidgetFPP->rowCount();
        m_ui->tableWidgetFPP->insertRow(row);

        m_ui->tableWidgetFPP->setItem(row, 0, new QTableWidgetItem());
        m_ui->tableWidgetFPP->item(row, 0)->setText(fpp.IP);
        m_ui->tableWidgetFPP->item(row, 0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        m_ui->tableWidgetFPP->setItem(row, 1, new QTableWidgetItem());
        m_ui->tableWidgetFPP->item(row, 1)->setText(fpp.Host);
        m_ui->tableWidgetFPP->item(row, 1)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        m_ui->tableWidgetFPP->setItem(row, 2, new QTableWidgetItem());
        m_ui->tableWidgetFPP->item(row, 2)->setText(fpp.Mode);
        m_ui->tableWidgetFPP->item(row, 2)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        m_ui->tableWidgetFPP->setItem(row, 3, new QTableWidgetItem());
        m_ui->tableWidgetFPP->item(row, 3)->setText(fpp.Version);
        m_ui->tableWidgetFPP->item(row, 3)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        m_ui->tableWidgetFPP->setItem(row, 4, new QTableWidgetItem());
        m_ui->tableWidgetFPP->item(row, 4)->setText(fpp.Status);
        m_ui->tableWidgetFPP->item(row, 4)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        m_ui->tableWidgetFPP->setItem(row, 5, new QTableWidgetItem());
        m_ui->tableWidgetFPP->item(row, 5)->setText("Click to Open");
        m_ui->tableWidgetFPP->item(row, 5)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        m_ui->tableWidgetFPP->setItem(row, 6, new QTableWidgetItem());
        m_ui->tableWidgetFPP->item(row, 6)->setText("Click to Test");
        m_ui->tableWidgetFPP->item(row, 6)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        m_ui->tableWidgetFPP->setItem(row, 7, new QTableWidgetItem());
        m_ui->tableWidgetFPP->item(row, 7)->setText("Click to Stop Test");
        m_ui->tableWidgetFPP->item(row, 7)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
}

bool MainWindow::contains_FPP(FPP_Data const& fpp) 
{
    return std::find_if(m_FPP_List.begin(), m_FPP_List.end(), [&fpp](auto const& s) {
        return s.IP == fpp.IP && s.Host == fpp.Host;
        }) != m_FPP_List.end();
}

void MainWindow::add_FPP(FPP_Data fpp) 
{
    //look if already in list
    if (contains_FPP(fpp)) {
        return;
    }

    m_FPP_List.push_back(std::move(fpp));
}

void MainWindow::saveList()
{
    QStringList ips;
    for (auto const& fpp : m_FPP_List)
    {
        ips.append(fpp.IP);
    }

    QSettings appsettings("FPP_Test", "FPP_Test");
    appsettings.setValue("IP_Addresses", ips.join(","));
}

void MainWindow::loadList()
{
    QSettings appsettings("FPP_Test", "FPP_Test");
    QString allIPs = appsettings.value("IP_Addresses", "").toString();
    QStringList ips = allIPs.split(",", QString::SkipEmptyParts);
    
    for (auto const& ip: ips) {
        QString const url = QString("http://%1/api/system/info").arg(ip);
        m_manager->get(QNetworkRequest(QUrl(url)));
    }

    if (!ips.empty()) {
        QTimer::singleShot(5000, this, SLOT(refresh_statuses()));
    }
}

void MainWindow::refresh_statuses()
{
    if (refesh_running)
    {
        return;
    }

    refesh_running = true;
    for (int i =0; i< m_ui->tableWidgetFPP->rowCount(); ++i)
    {
        auto ip = m_ui->tableWidgetFPP->item(i, 0)->text();
        QString const urlSt = QString("http://%1/api/system/status").arg(ip);
        QNetworkAccessManager manager;


        QUrl url(urlSt);
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

        QNetworkReply* networkReply = manager.get(request);

        QTime timer;
        timer.start();

        while (!networkReply->isFinished()) {
            // Check for a timeout condition (3s)
            if (timer.elapsed() >= (3 * 1000))
            {
                networkReply->abort();
                continue;
            }
            QApplication::processEvents();
        }

        auto reply = (QString(networkReply->readAll()));
		auto tt = reply.toStdString();
        if (tt.empty()) {
            m_ui->tableWidgetFPP->item(i, 4)->setText("No Response");
            continue;
		}
        nlohmann::json doc = nlohmann::json::parse(tt);
        {
            //auto const& json = doc.object();
            //if (json.contains("fppd") && json["fppd"].isString()) {
             //   FPPD = json["fppd"].toString();
            //}

            if (doc.contains("status_name") && doc["status_name"].is_string()) {
                m_ui->tableWidgetFPP->item(i, 4)->setText(doc["status_name"].get<QString>());
            }
        }

    }

    refesh_running = false;
}

void MainWindow::on_tableWidgetFPP_cellClicked(int row, int column)
{
    if (column == 5) 
    {
        auto const ip = m_ui->tableWidgetFPP->item(row, 0)->text();
        QDesktopServices::openUrl(QUrl("http://"+ip));
    }

    if (column == 6)
    {
        auto const ip = m_ui->tableWidgetFPP->item(row, 0)->text();
        setTesting(ip, 1);
    }

    if (column == 7)
    {
        auto const ip = m_ui->tableWidgetFPP->item(row, 0)->text();
        setTesting(ip, 0);
    }
}


void MainWindow::setTesting(QString const& ip, int enable)
{
    nlohmann::json json;

    json["cycleMS"] = 500;
    json["enabled"] = enable;
    json["channelSet"] = "1-1048576";
    json["channelSetType"] = "channelRange";

    json["mode"] = "RGBChase";
    json["subMode"] = "RGBChase-RGB";
    json["colorPattern"] = "FF000000FF000000FF";
    
    //QJsonDocument doc(json);
    QString strJson(json.dump().c_str());

    QString const data = "command=setTestMode&data=" + strJson;
    //QByteArray const postDataSize = QByteArray::number(data.size());

    QString const urlSt = QString("http://%1/fppjson.php?%2").arg(ip).arg(data);
    QNetworkAccessManager manager;

    QUrl url(urlSt,QUrl::ParsingMode::TolerantMode);

    //hack to prevent encoding?
    url.setQuery(url.query(QUrl::FullyDecoded), QUrl::DecodedMode);
 
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QNetworkReply* networkReply = manager.post(request,"");

    QTime timer;
    timer.start();

    while (!networkReply->isFinished()) {
        // Check for a timeout condition (3s)
        if (timer.elapsed() >= (3 * 1000))
        {
            networkReply->abort();
            continue;
        }
        QApplication::processEvents();
    }

    auto reply = (QString(networkReply->readAll()));

    QTimer::singleShot(2000, this, SLOT(refresh_statuses()));
}