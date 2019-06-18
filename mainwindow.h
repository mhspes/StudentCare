#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QtDebug>
#include <time.h>
#include <stdlib.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QSqlDatabase db;

    void connClose(){
        db.close();
        db.removeDatabase(QSqlDatabase::defaultConnection);
    }
    // Check if the connection is established
    bool connOpen(){
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("../StudentCare/database/studentcare.db");

        if (!db.open()){
            qDebug()<< ("Failed to open database.");
            return false;
        }else{
            qDebug()<< ("Connected.");
            return true;
        }
    }


public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:

    void on_loginButton_clicked();

    void on_btn_reset_database_clicked();

    void on_btn_close_clicked();

    void on_btn_main_menu_clicked();

    void on_btn_register_std_clicked();

    void on_btn_add_task_std_clicked();

    void on_search_btn_std_clicked();

    void on_add_btn_clicked();

    void on_search_assgn_clicked();

    void on_btn_grade_0_clicked();

    void on_btn_grade_1_clicked();

    void on_search_grade_btn_clicked();

    void on_changetoStd_clicked();

private:
    int user_id;
    QString user_role;

public:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
