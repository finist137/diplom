#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "coordinates.h"
#include "string.h"
#include <QFileDialog>
#include <QtGui>

#define PI 3.141592653
#define BOLZMAN 1.38/100000000000000000000000

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
   ui->setupUi(this);

   load_pictures();

   load_start_settings();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{

    coordinates Geo_Barnaul(0, 0, 0, 0),  NIP(0,0,0,0), Vector_KA(1,2,3,4);

    float pitch = 1, yaw = 0, wave_length = 0, frequency = ui->frequency->text().toFloat(), DN[25*91*2],
            latitude = ui->latitude->text().toFloat(), longitude = ui->longitude->text().toFloat(), KY_trans = 0, trans_D,
            P_trans = 10*log((ui->power_of_trans->text()).toFloat()),S_ant = (ui->earth_S_of_ant->text()).toFloat(),
            loss_AFU = (ui->loss_AFU->text()).toFloat(), loss_accurate = (ui->loss_accurate->text()).toFloat(),
            loss_atm = (ui->loss_atm->text()).toFloat(), loss_polar = (ui->loss_polar->text()).toFloat(),
            earth_T = (ui->earth_T->text()).toFloat(), energy = (ui->energy->text()).toFloat(),
            informative = (ui->informative->toPlainText()).toFloat();

    int N = 1000;

    QVector<double> KND_Barnaul(N), time_KND(N), range_Bar(N), godograph_bar_X(N), godograph_bar_Y(N), energy_loss(530);

    QString to_read, adress_vector_KA, adress_angles, adress_DN, bar_KND_time1 = ui->bar_time_KND1->text(),
            bar_KND_time2 = ui->bar_time_KND2->text(), bar_KND1 = ui->bar_KND1->text(), bar_KND2 = ui->bar_KND2->text(),
            bar_range_time1 = ui->bar_time_range1->text(), bar_range_time2 = ui->bar_time_range2->text(),
            bar_range1 = ui->bar_range1->text(), bar_range2 = ui->bar_range2->text(),
            supply_time_1 = ui->supply_time_1->text(), supply_time_2 = ui->supply_time_2->text(),
            supply_KND_1 = ui->supply_KND_1->text(), supply_KND_2 = ui->supply_KND_2->text();

    QFile adresses_of_files("N.U/adress.txt");;
    QFile file_range_Zhelez("N.U/Barnaul.txt");
    QFile file_KND("N.U/KND.txt");

    wave_length = 300/frequency;//скорость света делим на частоту

    KY_trans = 10*log(4*PI*S_ant/(wave_length*wave_length));//формула вычисления КУ передатчика

    trans_D = KY_trans - 10*log(earth_T);//добротность

    ui->wave_length->clear();
    ui->earth_D->clear();
    ui->KY_of_trans->clear();

    ui->wave_length->insertPlainText(QString::number(wave_length) + " м");
    ui->earth_D->insertPlainText(QString::number(trans_D) + " дБ/К");
    ui->KY_of_trans->insertPlainText(QString::number(KY_trans) + " дБ");

    ui->mistakes->clear();

    if(!adresses_of_files.open(QIODevice::ReadOnly)){
         ui->mistakes->insertPlainText("Не найден файл адресов.");
         return;
     }

     QTextStream adress_read(&adresses_of_files);

     adress_read>>adress_vector_KA;
     adress_read>>adress_angles;
     adress_read>>adress_DN;
     adresses_of_files.close();

     QFile file_vector_KA(adress_vector_KA);
     QFile file_angles(adress_angles);
     QFile file_DN(adress_DN);

    if (!file_KND.open(QIODevice::WriteOnly)) {

        ui->mistakes->insertPlainText("Не найден файл КНД.");
        return;
    }

    if (!file_range_Zhelez.open(QIODevice::ReadOnly)) {

        ui->mistakes->insertPlainText("Не найден файл расстояния до Барнаула.");
        return;
    }

    if (!file_vector_KA.open(QIODevice::ReadOnly)) {

        ui->mistakes->insertPlainText("Не найден вектор КА.");
        return;
    }
    else {
        ui->adress_vector_KA->clear();
        ui->adress_vector_KA->insertPlainText(adress_vector_KA);
    }

    if (!file_angles.open(QIODevice::ReadOnly)) {

        ui->mistakes->insertPlainText("Не найдены углы.");
        return;
    }
    else {
        ui->adress_angles->clear();
        ui->adress_angles->insertPlainText(adress_angles);
    }

    if (!file_DN.open(QIODevice::ReadOnly)) {

        ui->mistakes->insertPlainText("Не найдена диаграмма направленности.");
        return;
    }
    else{
        ui->adress_DN->clear();
        ui->adress_DN->insertPlainText(adress_DN);
    }

    QTextStream vector_KA_read(&file_vector_KA);
    QTextStream angles_read(&file_angles);;
    QTextStream DN_read(&file_DN);
    QTextStream stream_range_Zhelez(&file_range_Zhelez);
    QTextStream stream_KND(&file_KND);

    // перевод во вращаюшуюся гринвичскую СК из географических координат
    Geo_Barnaul.From_GeoSK_to_Grinvich(longitude, latitude, 0);
    NIP = Geo_Barnaul;

    for(int j = 0; j < 25*91; j++) {DN_read>>to_read; DN[j] = to_read.toFloat();}

    for(int i = 0;i<528;++i)
       {
           Vector_KA.read_KA_vector(vector_KA_read);

           Vector_KA.spin_Z(63.3);
           Vector_KA.spin_Y(-46);
           Vector_KA.spin_X(-60.7);
           Vector_KA.Set_Coordinates(Vector_KA.Get_Z()*1000, Vector_KA.Get_X()*1000, Vector_KA.Get_Y()*1000, 1);

           angles_read>>to_read;
           pitch = to_read.toFloat();

           angles_read>>to_read;
           yaw = to_read.toFloat();

           angles_read>>to_read;
           stream_range_Zhelez>>to_read;

           Geo_Barnaul = NIP ;

           //в инерциальную СК на момент старта
           Geo_Barnaul.spin_Z(-i*360/(24*3600));

           //в инерциальную стартовую СК три поворота

           //1) вокруг OZ на долготу
           Geo_Barnaul.spin_Z(63.3);

           //2) вокруг OY на широту
           Geo_Barnaul.spin_Y(-46);

           //3) вокруг OX на азимут разворота орбиты
           Geo_Barnaul.spin_X(-60.7);

           //4) совмещаем оси
           Geo_Barnaul.Set_Coordinates(Geo_Barnaul.Get_Z(), Geo_Barnaul.Get_X(), Geo_Barnaul.Get_Y(), i);
           range_Bar[i] = Vector_KA.Get_Range(Geo_Barnaul.Get_X(), Geo_Barnaul.Get_Y(), Geo_Barnaul.Get_Z());

           //в связанную СК КА
           Geo_Barnaul.spin_Y(yaw);

           Geo_Barnaul.spin_Z(pitch);

           Geo_Barnaul.transfer_SK(Vector_KA.Get_X(),Vector_KA.Get_Y(), Vector_KA.Get_Z());

           if((i > (bar_KND_time1.toInt()+1)))
                if(i < bar_KND_time2.toInt()){
                    godograph_bar_X[i - bar_KND_time1.toInt()] = Geo_Barnaul.Get_X()/(Geo_Barnaul.Get_Range(0,0,0)*1000);
                    godograph_bar_Y[i - bar_KND_time1.toInt()] = Geo_Barnaul.Get_Y()/(Geo_Barnaul.Get_Range(0,0,0)*1000);

                    if(godograph_bar_X[i - bar_KND_time1.toInt()] == 0){
                        godograph_bar_X[i - bar_KND_time1.toInt()] = godograph_bar_X[i - bar_KND_time1.toInt()-1];;
                    }
                    if(godograph_bar_Y[i - bar_KND_time1.toInt()] == 0){
                        godograph_bar_Y[i - bar_KND_time1.toInt()] = godograph_bar_Y[i - bar_KND_time1.toInt()-1];
                    }
           }

           KND_Barnaul[i] = Geo_Barnaul.give_me_KY(DN);

           time_KND[i] = i;

           energy_loss[i] = 10*log(KND_Barnaul[i]) + P_trans + trans_D - 10*log(informative) - energy - 10*log(BOLZMAN) -
                   loss_accurate - loss_AFU - loss_atm - loss_polar - 20*log(4*PI*range_Bar[i]/(wave_length)) - 60;

           stream_KND<<KND_Barnaul[i]<<"\r\n";

      }

    make_graph(ui->graph_KND_Barnaul, KND_Barnaul, time_KND, bar_KND_time1.toFloat(),
               bar_KND_time2.toFloat(), bar_KND1.toFloat(), bar_KND2.toFloat(), "время, сек", "КНД, дБ" );

    make_graph(ui->graph_range_Barnaul, range_Bar, time_KND, bar_range_time1.toFloat(),
               bar_range_time2.toFloat(), bar_range1.toFloat(), bar_range2.toFloat(), "время, сек", "расстояние, км");

    make_graph(ui->energ_supply, energy_loss, time_KND, supply_time_1.toFloat(),supply_time_2.toFloat(),
               supply_KND_1.toFloat(), supply_KND_2.toFloat(), "время, сек", "энергетический запас радиолинии, дБ");

    make_godograph(ui->godograph_bar, 1, -1, 1, -1, DN, godograph_bar_X, godograph_bar_Y);

    set_settings();

    file_vector_KA.close();
    file_angles.close();
    file_DN.close();
    file_KND.close();
}

//получение адреса ВЕКТОРА КА
void MainWindow::on_Button_KA_clicked()
{
    QString fileName_DATA;
    QString adress_vector_KA, adress_pitch, adress_yaw, adress_DN;
    fileName_DATA = QFileDialog::getOpenFileName(this, tr("Open File"),"/home",tr("DATA (*.txt)"));
    QFile file(fileName_DATA);
    ui->adress_vector_KA->clear();

    if (!file.open(QIODevice::ReadOnly)) {

        ui->adress_vector_KA->insertPlainText("Файл не открыт");
        return;
    }

    ui->adress_vector_KA->insertPlainText(fileName_DATA);
    file.close();

   QFile adresses_of_files("N.U/adress.txt");

    if(!adresses_of_files.open(QIODevice::ReadWrite)){

        ui->mistakes->insertPlainText("Не найден файл адресов.");
        return;

    }

    QTextStream adress_read(&adresses_of_files);

    adress_read>>adress_vector_KA;
    adress_read>>adress_pitch;
    adress_read>>adress_DN;
    adresses_of_files.close();

    adresses_of_files.open(QIODevice::WriteOnly);
    QTextStream adress_write(&adresses_of_files);

    adress_vector_KA = fileName_DATA;

    adress_write<<adress_vector_KA<<"\r\n";
    adress_write<<adress_pitch<<"\r\n";
    adress_write<<adress_DN<<"\r\n";
    adresses_of_files.close();

}

//кнопка получения адреса файла с углами КА
void MainWindow::on_Button_pitch_clicked()
{
    QString fileName_DATA;
    QString adress_vector_KA, adress_angles, adress_DN;
    fileName_DATA = QFileDialog::getOpenFileName(this, tr("Open File"),"/home",tr("DATA (*.txt)"));
        QFile file(fileName_DATA);
        ui->adress_angles->clear();
        if (!file.open(QIODevice::ReadOnly)) {

            ui->adress_angles->insertPlainText("Файл не открыт");
            return;
        }
    ui->adress_angles->insertPlainText(fileName_DATA);
    file.close();

    QFile adresses_of_files("N.U/angles.txt");

    if(!adresses_of_files.open(QIODevice::ReadWrite)){

        ui->mistakes->insertPlainText("Не найден файл адресов.");
        return;

    }

    QTextStream adress_read(&adresses_of_files);

    adress_read>>adress_vector_KA;
    adress_read>>adress_angles;
    adress_read>>adress_DN;
    adresses_of_files.close();

    adresses_of_files.open(QIODevice::WriteOnly);
    QTextStream adress_write(&adresses_of_files);

    adress_angles = fileName_DATA;

    adress_write<<adress_vector_KA<<"\r\n";
    adress_write<<adress_angles<<"\r\n";
    adress_write<<adress_DN<<"\r\n";
    adresses_of_files.close();
}

//кнопка получения адреса файла ДН
void MainWindow::on_Button_DN_clicked()
{
    QString fileName_DATA;
    QString adress_vector_KA, adress_pitch, adress_yaw, adress_DN;
    fileName_DATA = QFileDialog::getOpenFileName(this, tr("Open File"),"/home",tr("DATA (*.txt)"));
    QFile file(fileName_DATA);
    ui->adress_DN->clear();
    if (!file.open(QIODevice::ReadOnly)) {
            ui->adress_DN->insertPlainText("Файл не открыт");
            return;
        }
    ui->adress_DN->insertPlainText(fileName_DATA);
    file.close();

    QFile adresses_of_files("N.U/adress.txt");
    if(!adresses_of_files.open(QIODevice::ReadWrite)){

        ui->mistakes->insertPlainText("Не найден файл адресов.");
        return;

    }

    QTextStream adress_read(&adresses_of_files);

    adress_read>>adress_vector_KA;
    adress_read>>adress_pitch;
    adress_read>>adress_DN;
    adresses_of_files.close();

    adresses_of_files.open(QIODevice::WriteOnly);
    QTextStream adress_write(&adresses_of_files);

    adress_DN = fileName_DATA;

    adress_write<<adress_vector_KA<<"\r\n";
    adress_write<<adress_pitch<<"\r\n";
    adress_write<<adress_DN<<"\r\n";
    adresses_of_files.close();

}

void MainWindow::on_help_clicked()
{
    QDesktopServices::openUrl(QUrl("Help.pdf"));
}

void MainWindow::on_exit_clicked()
{
    QApplication::quit();
}

void MainWindow::on_save_KND_clicked()
{
    QString adress_KND = QFileDialog :: getSaveFileName(0,"Save KND",QDir::currentPath(), "Notepad (*.txt);;All files (*.*)");
    QString KND;
    QString bar_range_time1 = ui->bar_time_KND1->text(), bar_range_time2 = ui->bar_time_KND2->text();

    QFile file(adress_KND);
    file.open(QIODevice::WriteOnly);
    QTextStream save_KND(&file);

    QFile file_KND("N.U/KND.txt");
    file_KND.open(QIODevice::ReadOnly);
    QTextStream read_KND(&file_KND);

    for(int i = 0; i < 527; i++){

    read_KND>>KND;
        if(i>bar_range_time1.toInt())
            if(i<bar_range_time2.toInt())
                save_KND<<KND<<"\r\n";
    }

    file.close();
    file_KND.close();
}

void MainWindow::make_graph(QCustomPlot* graph_KND, QVector<double> KND,
                            QVector<double> time, int x1_range, int x2_range,
                            int y1_range, int y2_range, char* axe_X, char* axe_Y){

    graph_KND->clearGraphs();//Если нужно, то очищаем все графики

    //Добавляем один график в widget
    graph_KND->addGraph();

    //Говорим, что отрисовать нужно график по нашим двум массивам x и y
    graph_KND->graph(0)->setData(time, KND);

    graph_KND->graph(0)->setPen(QColor(50, 50, 50, 255));//задаем цвет точки
    //ui->widget->graph(0)->setLineStyle(QCPGraph::lsNone);//убираем линии
    //формируем вид точек
    graph_KND->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));

    //Подписываем оси Ox и Oy
    graph_KND->xAxis->setLabel(axe_X);
    graph_KND->yAxis->setLabel(axe_Y);

    //Установим область, которая будет показываться на графике
    graph_KND->xAxis->setRange(x1_range, x2_range);//Для оси Ox
    graph_KND->yAxis->setRange(y1_range, y2_range);//Для оси Oy

    //И перерисуем график на нашем widget
    graph_KND->replot();
}

void MainWindow::make_godograph(QCustomPlot* graph_KND, int x1_range, int x2_range, int y1_range,
                                int y2_range, const float* DN, QVector<double> godograph_X,
                                QVector<double> godograph_Y){

    double  DN_precise[360*460];
    int k, a, b, l = 0, m = 0, n05 = 0, n1 = 0, n4 = 0, HH = 0, n6 = 0, o = 0;
    float phi, theta, j;
    double Kd = 0,
          Kc = 0;

    QVector<double> X05(1000), Y05(1000), X1(1000), Y1(1000), X4(1000), Y4(1000), X6(1000), Y6(1000), Circle_Y_minus(2100), Circle_X(2100), Circle_Y_plus(2100);

    //создаем график окружности

    for(j = -1; j < 1; j = j + 0.01){
        Circle_Y_plus[o] = sqrt(1 - (j*j));
        Circle_Y_minus[o] = -Circle_Y_plus[o];
        Circle_X[o] = j;
        o++;
    }

    Circle_Y_plus[o] = 0;
    Circle_X[o] = 1;



    for(phi = 0; phi < 360; phi = phi + 3.6)
     {
        m = 0;
        a = phi/14.4;// номер сечения по phi

         for(theta = 0; theta < 180; theta = theta + 1)
         {

            b = theta/2;// номер сечения по theta
            Kc = (phi - 14.4*a)*(DN[(a+1)*91 + b] - DN[a*91 + b])/14.4 + DN[a*91 + b];

            Kd = (phi - 14.4*a)*(DN[(a+1)*91 + b + 1] - DN[a*91 + b + 1])/14.4 + DN[a*91 + b + 1];
            HH = l;
            DN_precise[l] = (theta - 2*b)*(Kd - Kc)/2 + Kc;

            m++;
            l++;
        }
     }

    for(int i = 0; i < (100*180); i++){

        k = i/180;
        if((i - k*180) < 90){
            if(DN_precise[i] > 0.47){
                if(DN_precise[i]<0.53){

                    X05[n05] = sin((i - k*180)*3.14159265/180)*cos(3.6*k*3.14159265/180);
                    Y05[n05] = sin((i - k*180)*3.14159265/180)*sin(3.6*k*3.14159265/180);
                    n05++;
            }
                if(DN_precise[i] > 1.9)
                    if(DN_precise[i] < 2.1){
                        X1[n1] = sin((i - k*180)*3.14159265/180)*cos(3.6*k*3.14159265/180);
                        Y1[n1] = sin((i - k*180)*3.14159265/180)*sin(3.6*k*3.14159265/180);
                        n1++;
                    }

                if(DN_precise[i] > 3.9)
                    if(DN_precise[i] < 4.1){

                        X4[n4] = sin((i - k*180)*3.14159265/180)*cos(3.6*k*3.14159265/180);
                        Y4[n4] = sin((i - k*180)*3.14159265/180)*sin(3.6*k*3.14159265/180);
                        n4++;
                    }

                if(DN_precise[i] > 5.9) {
                    if(DN_precise[i] < 6.1){

                        X6[n6] = sin((i - k*180)*3.14159265/180)*cos(3.6*k*3.14159265/180);
                        Y6[n6] = sin((i - k*180)*3.14159265/180)*sin(3.6*k*3.14159265/180);
                        n6++;
                    }
                }
            }
        }
    }

    graph_KND->clearGraphs();//Если нужно, то очищаем все графики

    //Добавляем один график в widget
    graph_KND->addGraph();
    graph_KND->addGraph();
    graph_KND->addGraph();
    graph_KND->addGraph();
    graph_KND->addGraph();
    graph_KND->addGraph();
    graph_KND->addGraph();


    //Говорим, что отрисовать нужно график по нашим двум массивам x и y
    graph_KND->graph(0)->setData(X6, Y6);
    graph_KND->graph(1)->setData(X4, Y4);
    graph_KND->graph(2)->setData(X1, Y1);
    graph_KND->graph(3)->setData(X05, Y05);
    graph_KND->graph(4)->setData(godograph_X, godograph_Y);
    graph_KND->graph(5)->setData(Circle_X, Circle_Y_plus);
    graph_KND->graph(6)->setData(Circle_X, Circle_Y_minus);

    graph_KND->graph(0)->setPen(QColor(0, 0, 255, 255));//задаем цвет точки
    graph_KND->graph(1)->setPen(QColor(210, 105, 30, 255));//задаем цвет точки
    graph_KND->graph(2)->setPen(QColor(30, 144, 255, 255));//задаем цвет точки
    graph_KND->graph(3)->setPen(QColor(34, 139, 34, 255));//задаем цвет точки
    graph_KND->graph(4)->setPen(QColor(255, 0, 0, 255));//задаем цвет точки
    graph_KND->graph(5)->setPen(QColor(192, 192, 192, 255));//задаем цвет точки
    graph_KND->graph(6)->setPen(QColor(192, 192, 192, 255));//задаем цвет точки

    graph_KND->graph(0)->setLineStyle(QCPGraph::lsNone);//убираем линии
    graph_KND->graph(1)->setLineStyle(QCPGraph::lsNone);//убираем линии
    graph_KND->graph(2)->setLineStyle(QCPGraph::lsNone);//убираем линии
    graph_KND->graph(3)->setLineStyle(QCPGraph::lsNone);//убираем линии
    graph_KND->graph(4)->setLineStyle(QCPGraph::lsNone);//убираем линии
    //формируем вид точек
    graph_KND->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
    graph_KND->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
    graph_KND->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
    graph_KND->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 4));
    graph_KND->graph(4)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 7));
    graph_KND->graph(5)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));
    graph_KND->graph(6)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 2));

    //Установим область, которая будет показываться на графике
    graph_KND->xAxis->setRange(x1_range, x2_range);//Для оси Ox
    graph_KND->yAxis->setRange(y1_range, y2_range);//Для оси Oy

    //И перерисуем график на нашем widget
    graph_KND->replot();
}

void MainWindow::load_pictures(){
    QPixmap antenna("N.U\\1.jpg"),color05("N.U\\color05.png"), color1("N.U\\color1.png"),
            color4("N.U\\color4.png"), color6("N.U\\color6.png");

     ui->picture->setPixmap( antenna );
     ui->color1->setPixmap( color1 );
     ui->color05->setPixmap( color05 );
     ui->color4->setPixmap( color4 );
     ui->color6->setPixmap( color6 );
}

void MainWindow:: load_start_settings(){

    QString frequency, informative, AFU_lost, polar_lost, precision_lost, atmosphere_lost,
    PRD_power, T_of_PRM, S_of_PRM, energy, latitude, longitude;

    QFile file_options_of_launching("N.U/options_of_launching.txt");
    QTextStream options_read(&file_options_of_launching);
    ui->mistakes->clear();

     if (!file_options_of_launching.open(QIODevice::ReadOnly)) {

         ui->mistakes->insertPlainText("Не найдены начальные настройки запуска");
         return;
     }

     else {

        options_read>>frequency;
        options_read>>informative;
        options_read>>AFU_lost;
        options_read>>polar_lost;
        options_read>>precision_lost;
        options_read>>atmosphere_lost;
        options_read>>PRD_power;
        options_read>>T_of_PRM;
        options_read>>S_of_PRM;
        options_read>>energy;
        options_read>>latitude;
        options_read>>longitude;

        ui->frequency->clear();
        ui->frequency->insert(frequency);

        ui->informative->clear();
        ui->informative->insertPlainText(informative);

        ui->loss_AFU->clear();
        ui->loss_AFU->setText(AFU_lost);

        ui->loss_polar->clear();
        ui->loss_polar->setText(polar_lost);

        ui->loss_accurate->clear();
        ui->loss_accurate->setText(precision_lost);

        ui->loss_atm->clear();
        ui->loss_atm->setText(atmosphere_lost);

        ui->power_of_trans->clear();
        ui->power_of_trans->setText(PRD_power);

        ui->earth_T->clear();
        ui->earth_T->setText(T_of_PRM);

        ui->earth_S_of_ant->clear();
        ui->earth_S_of_ant->setText(S_of_PRM);

        ui->energy->clear();
        ui->energy->setText(energy);

        ui->latitude->clear();
        ui->latitude->setText(latitude);

        ui->longitude->clear();
        ui->longitude->setText(longitude);

        file_options_of_launching.close();

     }
}

void MainWindow::set_settings() {

        QFile file_options_of_launching("N.U/options_of_launching.txt");

        QTextStream stream_options_save(&file_options_of_launching);

        if (!file_options_of_launching.open(QIODevice::WriteOnly)) {

            ui->mistakes->insertPlainText("Не найдены начальные настройки запуска");
            return;
        }

        else {

            stream_options_save<<ui->frequency->text();
            stream_options_save<<"\r\n"<<(ui->informative->toPlainText()).toFloat();
            stream_options_save<<"\r\n"<<(ui->loss_AFU->text()).toFloat();
            stream_options_save<<"\r\n"<<(ui->loss_polar->text()).toFloat();
            stream_options_save<<"\r\n"<<(ui->loss_accurate->text()).toFloat();
            stream_options_save<<"\r\n"<<(ui->loss_atm->text()).toFloat();
            stream_options_save<<"\r\n"<<(ui->power_of_trans->text()).toFloat();
            stream_options_save<<"\r\n"<<(ui->earth_T->text()).toFloat();
            stream_options_save<<"\r\n"<<(ui->earth_S_of_ant->text()).toFloat();
            stream_options_save<<"\r\n"<<(ui->energy->text()).toFloat();
            stream_options_save<<"\r\n"<<ui->latitude->text();
            stream_options_save<<"\r\n"<<ui->longitude->text();

        }
        file_options_of_launching.close();
}
