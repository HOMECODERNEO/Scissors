#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <Includes.h>

class VersionChecker : public QObject{
    Q_OBJECT

public:
    explicit VersionChecker(QObject *parent = nullptr);

    void check();

private:
    void onReplyFinished(QNetworkReply *reply);
    QString extractVersion(const QString &input);
    bool isNewerVersionAvailable(const QString &currentVersion, const QString &newVersion);

signals:
    void ShowPopup(QString, QString, int, QString);

private:
    QNetworkAccessManager *_networkChecker;
};

#endif // VERSIONCHECKER_H
