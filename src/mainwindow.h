#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "fpp_data.h"

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSettings>

#include "spdlog/spdlog.h"
#include "spdlog/common.h"

#include <memory>
#include <atomic>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButtonAdd_clicked();
    void reply_finished(QNetworkReply *reply);

    void on_tableWidgetFPP_cellDoubleClicked(int row, int column);

    void refresh_statuses();

    void on_tableWidgetFPP_cellClicked(int row, int column);

    void loadList();

private:
    Ui::MainWindow *m_ui;
    QNetworkAccessManager *m_manager;

    std::shared_ptr<spdlog::logger> m_logger{ nullptr };
    std::unique_ptr<QSettings> m_settings{ nullptr };
    QString m_appdir;

    std::vector<FPP_Data> m_FPP_List;
    std::atomic<bool> refesh_running{ false };

    void searchForOthers(FPP_Data const& fpp);
    void add_FPP(FPP_Data fpp);
    bool contains_FPP(FPP_Data const& fpp);

    void Read_FPP_JSON( QString const& json);

    void setTesting(QString const& ip, int enable);

    void refreshList();

    void saveList();


};

#endif // MAINWINDOW_H
