#ifndef EXPORTRESULT_H
#define EXPORTRESULT_H

#include "globals.h"

#include <QDialog>
#include <QTableWidget>

namespace Ui {
class ExportResult;
}

enum ExportType
{
    UNKNOWN = 0,
    CSV     = 1,
    EXCEL   = 2,
    JSON    = 3
};

class ExportResult : public QDialog
{
    Q_OBJECT

public:
    explicit ExportResult(QWidget* parent, QTableWidget* tableWidget, ViewMode viewMode);
    ~ExportResult();

signals:
    void exportResultError(const QString error) const;

private slots:
    void pushButtonClose_clicked();
    void pushButtonExport_clicked();

    void checkBoxJSON_toggled   (bool checked);
    void checkBoxCSV_toggled    (bool checked);
    void checkBoxExcel_toggled  (bool checked);
    void checkBoxTab_toggled    (bool checked);

private:
    Ui::ExportResult*   ui;
    QTableWidget*       m_tableWidget     { nullptr };
    const ViewMode      m_viewMode;
    QString             m_defaultType;
    bool                m_userChangedType { false };

    void connectObjects();

    void exportResult(ExportType type);

    void exportJSON (const QList<QMap<QString, QString>> valuesMapList);
    void exportCSV  (const QList<QMap<QString, QString>> valuesMapList);
    void exportExcel(const QList<QMap<QString, QString>> valuesMapList);

    QString getIndentStringForJSON() const;

    void setToDefault();
    void setFromDefault();
    void enableDefault(QString who);
};

#endif // EXPORTRESULT_H
