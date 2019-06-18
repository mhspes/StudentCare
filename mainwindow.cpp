#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <time.h>
#include <ctime>
#include <QMessageBox>

// Method for parsing date-string as YYYY/MM/DD-XX:XX:XX
std::string currentTime(){

    time_t now = time(NULL);
    struct tm tmstruct;
    char buf[100];
    tmstruct = *localtime(&now);
    strftime(buf,sizeof(buf), "%Y/%m/%d-%X",&tmstruct);
    return buf;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0); // Page 0 - login screen
    setWindowTitle("StudentCare");

    if (!connOpen()){
        ui->db_label_main->setText("Failed to open the database.");

    } else {
        ui->db_label_main->setText("Connected to database.");
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_loginButton_clicked()
{
    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();

    if (!connOpen()){
        return;
    }

    QSqlQuery query;

    if(query.exec("SELECT * FROM personnel WHERE username='"+username+"' AND password='"+password+"'")){
        int counter=0;
        while(query.next()){
            counter++;
        }
        if(counter==1){
            // Username-password combination found..
            query.exec("SELECT isTeacher, isStudent, id FROM personnel WHERE username = '"+username+"'");
            while(query.next()){
                int isTeacher = query.value(0).toInt();
                int isStudent = query.value(1).toInt();
                user_id = query.value(2).toInt();

                // Check if teacher or student
                if(isTeacher != 0){
                    ui->label_3->setText("Logging in..");               
                    ui->stackedWidget->setCurrentIndex(2); // Page 2 - teacher view
                    setWindowTitle("Teacher options");
                    ui->label_3->setText("");
             }
                else if(isStudent != 0){
                    ui->label_3->setText("Logging in..");
                    ui->stackedWidget->setCurrentIndex(1); // Page 1 - student view
                    setWindowTitle("Student options");
                    ui->label_3->setText("");
                } else {
                    ui->label_3->setText("Something went wrong.");
                }
            }
        }
        if(counter<1){
            ui->label_3->setText("Wrong username or password.");
        }
    }
}

void MainWindow::on_btn_close_clicked()
{
    this->close();
}


void MainWindow::on_btn_main_menu_clicked()
{
    setWindowTitle("StudentCare");
    ui->stackedWidget->setCurrentIndex(0);
}
/* Student operations
 * -Register to course
 * -Upload assignment to course-instance
 * -Check grading conditions
 * */

// Course registration
void MainWindow::on_btn_register_std_clicked()
{
    QString instance_id, student_id, qry;

    student_id = QString::number(user_id);
    instance_id = ui->id_course_std_0->text();

    if (!connOpen()){
        return;
    }

    QSqlQuery query;
    qDebug()<< "Querying values: " << student_id << instance_id;
    // Query for course registration
    qry = "INSERT INTO coursestudents (studentId,instanceId) values ('"+student_id+"','"+instance_id+"')";
    query.prepare(qry);

    // Inform whether the database operation was successful or not
    if(query.exec()){
        QMessageBox::information(this,tr("Successful operation"),tr("Successfully registered to the course."));
        connClose();
    } else {
        QMessageBox::critical(this,tr("Error occured!"),query.lastError().text());
    }
}

// Upload assignment to course-instance
void MainWindow::on_btn_add_task_std_clicked()
{
    QString student_id, instance_id, exercise, submit_date, exercise_id, comment, qry;

    student_id = QString::number(user_id);

    instance_id=ui->id_course_std_1->text();
    exercise=ui->task_name_std->text();
    comment=ui->comment_std->text();

    if (!connOpen()){
        return;
    }
    /* Generate a pseudorandom value for exercise_id
     * Value is just a dummy (1-10000) which easily collides with other entities */
    srand(time(NULL));
    int temp = rand() % 10000 + 1;
    exercise_id = QString::number(temp);
    submit_date = QString::fromStdString(currentTime());

    QSqlQuery query;
    qry = "INSERT INTO exercises (instanceId,studentId,exerciseId,uploadResource,uploadDate,comment) values ('"+instance_id+"','"+student_id+"','"+exercise_id+"','"+exercise+"','"+submit_date+"','"+comment+"')";
    query.prepare(qry);

    if(query.exec()){
        QMessageBox::information(this,tr("Successful operation"),tr("Succesfully uploaded resource."));
        connClose();
    } else {
        QMessageBox::critical(this,tr("Error occured!"),query.lastError().text());
    }
}

// Search for submitted exercises
void MainWindow::on_search_btn_std_clicked()
{
    QSqlQueryModel * modal = new QSqlQueryModel();
    QString course_id, student_id;

    course_id = ui->id_course_std_2->text();
    student_id = QString::number(user_id);

    if(!connOpen()){
        return;
    }

    QSqlQuery * qry = new QSqlQuery(db);
    qry->prepare("SELECT uploadResource, comment, teacherComment, grade, gradeDate FROM exercises WHERE instanceId='"+course_id+"' AND studentID='"+student_id+"'");
    qry->exec();
    modal->setQuery(*qry);
    ui->tableView->setModel(modal);

    connClose();
    qDebug() << (modal->rowCount());
}

void MainWindow::on_search_grade_btn_clicked()
{
    QSqlQueryModel * modal = new QSqlQueryModel();
    QString student_id = QString::number(user_id);

    if(!connOpen()){
        return;
    }
    QSqlQuery * qry = new QSqlQuery(db);
    qry->prepare("SELECT instanceId, gradedate, grade FROM coursegrades WHERE studentId='"+student_id+"'");
    qry->exec();
    modal->setQuery(*qry);
    ui->tableView->setModel(modal);
    connClose();
}

/*  Teacher operations
 *  - Add or remove from course teachers-list
 *  - List course achievements
 *  - Grade exercises and courses
 * */

void MainWindow::on_add_btn_clicked()
{
    QString instance_id, teacher_id, option, qry;

    teacher_id = QString::number(user_id);

    if (!connOpen()){
        return;
    }
    instance_id = ui->id_course_tch_0->text();
    option = ui->choice_box->currentText();

    // Query for joining or leaving the course
    if(option=="Liity"){
        qry = "INSERT INTO courseteachers (instanceId,teacherId) values ('"+instance_id+"','"+teacher_id+"')";
    } else if (option=="Poistu"){
        qry = "DELETE FROM courseteachers WHERE instanceId = '"+instance_id+"' AND teacherId = '"+teacher_id+"'";
    } else {
    }
    QSqlQuery query;
    query.prepare(qry);

    // Inform whether the database operation was successful or not
 if(query.exec()){
        QMessageBox::information(this,tr("Successful operation"),tr("Query was successful."));
        connClose();
    } else {
        QMessageBox::critical(this,tr("Error occured!"),query.lastError().text());
    }
}

// Search for assignmnets
void MainWindow::on_search_assgn_clicked()
{
    QSqlQueryModel * modal = new QSqlQueryModel();
    QString instance_id, student_id;

    student_id = ui->id_std_tch_0->text();
    instance_id = ui->id_course_tch_1->text();

    if(!connOpen()){
        return;
    }
    QSqlQuery * qry = new QSqlQuery(db);

    qry->prepare("SELECT uploadResource, exerciseId, comment, teacherComment, grade, gradeDate FROM exercises WHERE instanceId='"+instance_id+"' AND studentId='"+student_id+"'");
    qry->exec();
    modal->setQuery(*qry);
    ui->tableView_2->setModel(modal);

    connClose();
    qDebug() <<(modal->rowCount());
}

// Grade single assignments
void MainWindow::on_btn_grade_0_clicked()
{
    QString teacher_id, instance_id, exercise_id, student_id, teacher_comment, grade, gradeDate, qry;

    teacher_id = QString::number(user_id);

    // Get values from text fields
    instance_id = ui->id_course_tch_2->text();
    exercise_id = ui->id_exercise_std_0->text();
    student_id = ui->id_std_tch_1->text();
    teacher_comment = ui->comment_tch->text();
    grade = ui->grade_0->text();

    gradeDate = QString::fromStdString(currentTime());

    if (!connOpen()){
        return;
    }
    qry = "UPDATE exercises SET teacherId='"+teacher_id+"', teacherComment='"+teacher_comment+"', grade='"+grade+"', gradeDate='"+gradeDate+"' WHERE exerciseId='"+exercise_id+"'";
    QSqlQuery query;
    query.prepare(qry);

    if(query.exec()){
        QMessageBox::information(this,tr("Successful operation"),tr("Query was successful."));
        connClose();
    } else {
        QMessageBox::critical(this,tr("Error occured!"),query.lastError().text());
    }

}

// Grade course-instances
void MainWindow::on_btn_grade_1_clicked()
{
    QString student_id, instance_id, gradeDate, grade, teacher_id, qry;
    teacher_id = QString::number(user_id);
    gradeDate = QString::fromStdString(currentTime());

    student_id = ui->id_std_tch_2->text();
    instance_id = ui->id_course_tch_3->text();
    grade = ui->grade_1->text();

    if (!connOpen()){
        return;
    }

    QSqlQuery query;
    qry = "INSERT INTO coursegrades (studentId,instanceId,gradedate,grade,teacherId) VALUES ('"+student_id+"','"+instance_id+"','"+gradeDate+"','"+grade+"','"+teacher_id+"')";
    query.prepare(qry);

    if(query.exec()){
        QMessageBox::information(this,tr("Successful operation"),tr("Query was successful."));
        connClose();
    } else {
        QMessageBox::critical(this,tr("Error occured!"),query.lastError().text());
    }

}
// Go to student-view
void MainWindow::on_changetoStd_clicked()
{
    setWindowTitle("Student options");
    ui->stackedWidget->setCurrentIndex(1); // Page 1 - student view
}


// Database initialization
void MainWindow::on_btn_reset_database_clicked()
{
    if (!connOpen()){
        return;
    }

    // Initializing the database
    QSqlQuery query;

    // Empty tables
    query.exec("DELETE FROM courses");
    query.exec("DELETE FROM programs");
    query.exec("DELETE FROM courseinstances");
    query.exec("DELETE FROM personnel");
    query.exec("DELETE FROM exercises");
    query.exec("DELETE FROM coursegrades");
    query.exec("DELETE FROM courseteachers");
    query.exec("DELETE FROM coursestudents");

    // Insert data for testing
    query.exec("INSERT INTO courses (name,shortName,description,credits) values ('Cryptography I','MATE0001','Basics of cryptographic protocols',5)");
    query.exec("INSERT INTO programs (id,name) values ('CS1','Computer Science')");
    query.exec("INSERT INTO courseinstances (instanceid,courseId,gradingRule) values ('MATE0001_2019','MATE0001','1-5')");
    query.exec("INSERT INTO personnel (firstNames,familyName,program,id,idString,password, username, isTeacher, isAdmin, isStudent) values ('Mihail','Hodorovski','CS1',777,'mhod','password','student',0,0,1)");
    query.exec("INSERT INTO personnel (firstNames,familyName,program,id,idString,password, username, isTeacher, isAdmin, isStudent) values ('Lasse','Bergroth','CS1',001,'lber','password','teacher',1,0,1)");

    QMessageBox::information(this,tr("Success!"),tr("Succesfully initialized database."));
}

