#ifndef DELETEQUERIESVIEWER_H
#define DELETEQUERIESVIEWER_H

#include <QDialog>

namespace Ui {
class DeleteQueriesViewer;
}

class DeleteQueriesViewer : public QDialog
{
    Q_OBJECT

public:
    explicit DeleteQueriesViewer(QWidget* parent = nullptr);
    ~DeleteQueriesViewer();

    void appendText(QString text);
    void clear();

private slots:
    void copyText();

signals:
    void message(QString);

private:
    Ui::DeleteQueriesViewer* ui;
};

#endif // DELETEQUERIESVIEWER_H
