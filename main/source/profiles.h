#ifndef PROFILES_H
#define PROFILES_H

#include "conditions.h"

#include <QDialog>
#include <QMap>
#include <QTreeWidgetItem>
#include <QJsonObject>

namespace Ui {
class Profiles;
}

class Profiles : public QDialog
{
    Q_OBJECT

    using ElementType = enum { ET_Profile, ET_Schema };

public:
    explicit Profiles(QWidget* parent, QString schema = {});
    ~Profiles();

    static bool saveCurrentProfile(QJsonObject jsonObj, Conditions& conditions);
    static bool saveCurrentSchema (QJsonObject jsonObj, Conditions& conditions);

    static QMap<int, QString> getProfiles();
    static QMap<int, QString> getSchemasForMenu ();
    static void               addSchema  (QString schema);

    static bool profileExists(QString profile);
    static QString getFirstProfile();

    static void updateProfileConnection(QString oldConnectionName, QString newConnectionName);

protected:
    bool eventFilter(QObject* obj, QEvent* event);

signals:
    void profileSelected(const QString profile, bool isDefault) const;
    void currentProfileUpdate() const;
    void messageMethodRequest(QString) const;

private slots:
    void pushButtonCancel_clicked();
    void pushButtonSelect_clicked();
    void pushButtonSave_clicked();
    void pushButtonCreateProfile_clicked();
    void lineEditNewProfile_textChanged(const QString& arg1);
    void treeView_clicked      (const QModelIndex& index);
    void treeView_doubleClicked(const QModelIndex& index);
    void pushButtonRemove_clicked();

private:
    Ui::Profiles* ui;
    QDialog*      m_dialog { nullptr };
    QString       m_profileSelected;
    QStringList   m_profiles;
    QStringList   m_schemas;
    QString       m_profilesDir;
    QString       m_currentFilename;
    QString       m_currentSchema;
    QString       m_currentEditingProfile;

    void    connectObjects ();
    void    createProfile  (QString profile);
    void    newProfile     ();
    QString getFileContent (QString filepath) const;
    void    expandSchema   (const QModelIndex& index, QString schema = {});

    static bool saveCurrent(ElementType elementType, QJsonObject jsonObj, Conditions& conditions);

    QPair<QPair<QString, QString>, int> getInfoFromIndex(const QModelIndex& index) const;
};

#endif // PROFILES_H
