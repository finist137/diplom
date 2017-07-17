#ifndef COORDINATES_H
#define COORDINATES_H

#include <QObject>
#include <QDebug>
#include <fstream>
#include <string>
#include <math.h>

using namespace std;

#define SEMI_AXIS_GEOID 6378245
#define COMPRESSION_GEOID 1/298.3

class coordinates : public QObject
{
    Q_OBJECT
private:
    float X,
          Y,
          Z;

    int time;
public:
    explicit coordinates(QObject *parent = 0);

    coordinates(){

        X = 0;
        Y = 0;
        Z = 0;
        time = 0;
    }

    coordinates(float x, float y, float z, int t){

        X = x;
        Y = y;
        Z = z;
        time = t;
    }
    float Get_X(){return X;}
    float Get_Y(){return Y;}
    float Get_Z(){return Z;}

    int Get_Time(){return time;}

    void Set_Coordinates(float x, float y, float z, int t){
        X = x;
        Y = y;
        Z = z;
        time = t;
    }

    void operator =(coordinates &SMTH){

        X = SMTH.X;
        Y = SMTH.Y;
        Z = SMTH.Z;
        time = SMTH.time;
    }
    void read_KA_vector(QTextStream& read_vector_KA){
        QString gg = "0.0";

        read_vector_KA>>gg;
        time = gg.toFloat();
        read_vector_KA>>gg;
        X = gg.toFloat();
        read_vector_KA>>gg;
        Y = gg.toFloat();
        read_vector_KA>>gg;
        Z = gg.toFloat();
        QString g1;
        read_vector_KA>>g1;
        read_vector_KA>>g1;
        read_vector_KA>>g1;
    }

    void spin_X(float angle){

        float Y_vrem1 = Y;
        Y = cos(angle*3.14159265/180)*Y_vrem1 + sin(angle*3.14159265/180)*Z;
        Z = -sin(angle*3.14159265/180)*Y_vrem1 + cos(angle*3.14159265/180)*Z;
    }

    void spin_Y(float angle){

        float X_vrem1 = X;
        X = cos(angle*3.14159265/180)*X_vrem1 - sin(angle*3.14159265/180)*Z;
        Z = sin(angle*3.14159265/180)*X_vrem1 + cos(angle*3.14159265/180)*Z;
    }

      void spin_Z(float angle){

           float X_vrem = X;
           X = cos(angle*3.14159265/180)*X_vrem + sin(angle*3.14159265/180)*Y;
           Y = -sin(angle*3.14159265/180)*X_vrem + cos(angle*3.14159265/180)*Y;

       }

     void transfer_SK(float x, float y, float z){
        X = -(X - x);
        Y = -(Y - y);
        Z = -(Z - z);
    }
     float Get_Range(float x, float y, float z){
         return sqrt((X-x)*(X-x) + (Y-y)*(Y-y) + (Z-z)*(Z-z))/1000;
     }

     void From_GeoSK_to_Grinvich(float longitude, float latitude, float hight){

            float eccentricity = 2*COMPRESSION_GEOID - COMPRESSION_GEOID*COMPRESSION_GEOID;
            float N = SEMI_AXIS_GEOID/sqrt(1 - eccentricity*sin(latitude*3.14159265/180)*sin(latitude*3.14159265/180));

            X = (N + hight)*cos(latitude*3.14159265/180)*cos(longitude*3.14159265/180);
            Y = (N + hight)*cos(latitude*3.14159265/180)*sin(longitude*3.14159265/180);
            Z = ((1 - eccentricity)*N + hight)*sin(latitude*3.14159265/180);
        }
     float give_me_KY(const float* DN){


            float phi = 0, theta = 0;

            theta = acos(Z/sqrt(X*X + Y*Y + Z*Z))*180/3.14159265;

            phi = atan(Y/X)*180/3.14159265;

            if(X < 0) {
                if(Y > 0) phi = 180 + phi;
                else phi = 180 + phi;
            }
            else {
                if(Y < 0) phi = 360 + phi;
            }

            float Kd = 0,
                  Kc = 0,
                  K = 0;

            int a = phi/14.4;
            int b = theta/2;

            Kc = (phi - 14.4*a)*(DN[(a+1)*91 + b] - DN[a*91 + b])/14.4 + DN[a*91 + b];

            Kd = (phi - 14.4*a)*(DN[(a+1)*91 + b + 1] - DN[a*91 + b + 1])/14.4 + DN[a*91 + b + 1];
            K = (theta - 2*b)*(Kc - Kd)/2 + Kc;

            return K;
            }


signals:

public slots:
};

#endif // COORDINATES_H
