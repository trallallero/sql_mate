#ifndef QUERYVIEWER_H
#define QUERYVIEWER_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class QueryViewer;
}

class QueryViewer : public QDialog
{
    Q_OBJECT

public:
    explicit QueryViewer(QWidget* parent = nullptr);
    QueryViewer(QWidget* parent, QString text);
    ~QueryViewer();

private slots:
    void pushButtonSave_clicked();
    void pushButtonClose_clicked();
    void pushButtonCopy_clicked();
    void textBrowser_textChanged();

signals:
    void messageMethodRequest(QString) const;

private:
    Ui::QueryViewer* ui;

    void connectObjects();
    void saveQuery();
};

#endif // QUERYVIEWER_H
