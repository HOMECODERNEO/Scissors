#include "versionchecker.h"

VersionChecker::VersionChecker(QObject *parent): QObject{parent}{
    _networkChecker = new QNetworkAccessManager(this);
    connect(_networkChecker, &QNetworkAccessManager::finished, this, &VersionChecker::onReplyFinished);
}

void VersionChecker::check(){
    QNetworkRequest request(QUrl("https://api.github.com/repos/HOMECODERNEO/Scissors/releases/latest"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    connect(_networkChecker, &QNetworkAccessManager::finished, this, &VersionChecker::onReplyFinished);

    _networkChecker->get(request);
}

void VersionChecker::onReplyFinished(QNetworkReply *reply){
    if(reply->error() == QNetworkReply::NoError){
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();

        QString newVersion = extractVersion(root.value("tag_name").toString());
        QString currentVersion = extractVersion(APPLICATION_VERSION);

        if(isNewerVersionAvailable(currentVersion, newVersion)){
            //Доступна новая версия
            emit ShowPopup("#NEW_VERSION_AVIABLE#: v" + newVersion, "v" + currentVersion, 0, "https://github.com/HOMECODERNEO/Scissors/releases");
        }

    }else{
        emit ShowPopup("ERROR: " + reply->errorString(), "", 3000, "");
    }

    reply->deleteLater();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////// CHECK FUNC
////////////////////////////////////////////////////////////////////////////////////////////////////////////// CHECK FUNC
////////////////////////////////////////////////////////////////////////////////////////////////////////////// CHECK FUNC

QString VersionChecker::extractVersion(const QString &input){
    QRegularExpression regex("v(\\d+\\.\\d+\\.\\d+)");
    QRegularExpressionMatch match = regex.match(input);

    if (match.hasMatch()){
        return match.captured(1);
    }

    return QString(); // Версия не найдена
}

bool VersionChecker::isNewerVersionAvailable(const QString &currentVersion, const QString &newVersion){

    QStringList currentParts = currentVersion.split(".");
    QStringList newParts = newVersion.split(".");

    for(int i = 0; i < qMin(currentParts.size(), newParts.size()); ++i){
        int currentPart = currentParts.at(i).toInt();
        int newPart = newParts.at(i).toInt();

        if(currentPart < newPart)
            return true;
        else if(currentPart > newPart)
            return false;
    }

    return newParts.size() > currentParts.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////// END
////////////////////////////////////////////////////////////////////////////////////////////////////////////// END
////////////////////////////////////////////////////////////////////////////////////////////////////////////// END
