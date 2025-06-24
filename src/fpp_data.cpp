
#include "FPP_Data.h"

#include "json-qt.hpp"
#include <QString>

void FPP_Data::readJson(nlohmann::json const& json)
{
    if (json.contains("HostName") && json["HostName"].is_string()) {
        Host = json["HostName"].get<QString>();
    }

    if (json.contains("Version") && json["Version"].is_string()) {
        Version = json["Version"].get<QString>();
    }

    if (json.contains("version") && json["version"].is_string()) {
        Version = json["version"].get<QString>();
    }

    if (json.contains("fppMode") && json["fppMode"].is_string()) {
        Mode = json["fppMode"].get<QString>();
    }

    if (json.contains("Mode") && json["Mode"].is_string()) {
        Mode = json["Mode"].get<QString>();
    }

    if (json.contains("Platform") && json["Platform"].is_string()) {
        Platform = json["Platform"].get<QString>();
    }

    if (json.contains("majorVersion") && json["majorVersion"].is_number_integer()) {
        Major = json["majorVersion"].get<int>();
    }

    if (json.contains("minorVersion") && json["minorVersion"].is_number_integer()) {
        Minor = json["minorVersion"].get<int>();
    }

    if (json.contains("IPs") && json["IPs"].is_array()) {
        if (json["IPs"].get<QStringList>().count() > 0) {
            IP = json["IPs"].get<QStringList>().last();
        }
    }

    if (json.contains("IP") && json["IP"].is_string()) {
        IP = json["IP"].get<QString>();
    }
}

void FPP_Data::SetStatus(nlohmann::json const& json)
{
    if (json.contains("fppd") && json["fppd"].is_string()) {
        FPPD = json["fppd"].get<QString>();
    }

    if (json.contains("status_name") && json["status_name"].is_string()) {
        Status = json["status_name"].get<QString>();
    }
}