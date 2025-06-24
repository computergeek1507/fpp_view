#pragma once

#include <QString>

#include <nlohmann/json.hpp>

struct FPP_Data
{
    FPP_Data() {}

    FPP_Data(nlohmann::json const& json)
    {
        readJson(json);
    }

    inline bool operator==(const FPP_Data& rhs)
    {
        return IP == rhs.IP && Host == rhs.Host;
    }

    //inline bool operator==(const QString& ip, const QString& host)
    //{
    //    return IP == ip && Host == host;
    //}

    QString IP;
    QString Host;
    QString Version;
    QString Mode;
    QString Platform;
    QString Status;
    QString FPPD;
    int Major{ -1 };
    int Minor{ -1 };

    void SetStatus(nlohmann::json const& json);

private:
    void readJson(nlohmann::json const& json);

};

