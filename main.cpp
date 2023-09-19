
#include <math.h>
#include <stdio.h>
#include "HEPTA_COM.h"

DigitalOut analogpin(PA_3);
HEPTA_COM com(PA_9,PA_10,9600);
Serial sat(USBTX,USBRX,9600);
DigitalOut LED(PB_1);
Serial gps(PA_9,PA_10,9600);


float gps_sat_get(float *lat,float *lon)
{
    //while(1){
    
    analogpin = 0;//gps
    //pc.baud(9600);
    //com.baud(9600);
    //GPS
    char msg[256];
    int ite = 0;
    int quality = 0;
    int stnum = 0;
    //int serial_check = 0;
    char GPGGA[128]={0};
    float time = 0;
    float latitude = 0;
    float longitude = 0;
    float hacu = 0;
    float altitude = 0;
    char ns,ew,aunit; 
    
    
    
    for(int i=0; i<6; i++){
        msg[i]=gps.getc();
        sat.putc(msg[i]);
    }
    
    //sat.printf("\r\n");
    if((msg[3]=='G')&(msg[4]=='G')&(msg[5]=='A')){
        //sat.printf("%c,%c,%c",msg[3],msg[4],msg[5]);
        //wait(1);
        for(int j=0; j<6; j++){//msgd?j==0??
            if(j==0){
                for(int i=5; i<256; i++){
                    msg[i]=gps.getc();
                    //sat.putc(msg[i]);
                    if(msg[i]=='\r'){
                        msg[i]=0;
                        break;
                    }
                }
                //sat.printf("\r\n");
            }
        }  
        
        
    //}
        //pc.printf("Get GPS Data");     
        if(sscanf(msg,"%[^,],%f,%f,%c,%f,%c,%d,%d,%f,%f,%c",GPGGA,&time,&latitude,&ns,&longitude,&ew,&quality,&stnum,&hacu,&altitude,&aunit)>=1){
            if(!(quality)){
                latitude=int(latitude/100)+(latitude-int(latitude/100)*100)/60;
                longitude=int(longitude/100)+(longitude-int(longitude/100)*100)/60;
                //serial_check = 0;
            }
            else{
                latitude=int(latitude/100)+(latitude-int(latitude/100)*100)/60;
                longitude=int(longitude/100)+(longitude-int(longitude/100)*100)/60;
                //serial_check = 1;
            }
            if(!(ns=='N')){
                latitude =-latitude;
            }
            if(!(ew=='E')){
                longitude = -longitude;
            }
            *lat=latitude;
            *lon=longitude;
            //pc.printf("%f,%f",latitude,longitude);
            //pc.printf("\r\n");
        }
        else {
            sat.printf("No Data");
            //serial_check=2;
        }
    *lat=latitude;
    *lon=longitude;
    } //if GGA
    
    else{
        *lat=0.0;
        *lon=0.0;
    }
    
    
    //} 
}

float gps_send(float lat,float lon){
    analogpin = 1;//xbee
    char buf[30];
    int i=0;
    snprintf(buf,30,"b%f,%fa",lat,lon);
    while(buf[i] != '\0'){
        sat.putc(buf[i]);
        i++;
        wait(0.2);
        //wait(0.5);
    }
    //sat.printf("\r\n");
    //wait(0.5);
}

// 緯度、経度をラジアンに変換する
double toRadian(double value) {
    return value * 3.141592 / 180.0;
}

double distance(double long_sat,double long_ht,double lat_sat,double lat_ht) {
    //長半径Rx[m], 短半径Ry[m]
    double Rx = 6378137.0;
    double Ry = 6356752.314245;
    //緯度の差Dy, 経度の差Dx
    double Dy = lat_ht - lat_sat;
    double Dx = long_ht - long_sat;
    //2点の緯度の平均
    double P = (lat_sat + lat_ht) / 2;
    //離心率
    double E = pow(((pow(Rx, 2.0) - pow(Ry, 2.0)) / pow(Rx, 2.0)), (0.5));
    //W
    double W = pow((1 - pow(E, 2.0) * pow(sin(P), 2.0)), (0.5));
    //子午線曲率半径M
    double M = Rx * (1 - pow(E, 2.0)) / pow(W, 3.0);
    //卯酉線曲線半径N
    double N = Rx / W;
    //距離D
    double D = pow(pow((Dy * M), 2.0) + pow((Dx * N * cos(P)), 2.0), 0.5);
    return D;
}

double direction(double long_sat, double long_ht, double lat_sat, double lat_ht) {
    //長半径Rx[m], 短半径Ry[m]
    double Rx = 6378137.0;
    double Ry = 6356752.314245;
    //緯度の差Dy, 経度の差Dx
    double Dy = lat_ht - lat_sat;
    double Dx = long_ht - long_sat;
    //satからhtへの角度出す
    double a = sin(Dx);
    double b = cos(lat_sat) * tan(lat_ht) - sin(lat_sat) * cos(Dx);
    double c = b/a;
    double phi = 90-atan(c) * 180.0 / 3.141592;
    if (phi < 0) {
        phi = 360 + phi;
    }
    else {
        phi = phi;
    }
    return phi;
}

int main() {
    //latは緯度,longは経度
    //それぞれ_satは缶サット, _htは人
    float lat=1.0, lon=5.0;
    gps_sat_get(&lat,&lon);
    //pc.printf("%f,%f\n\r",lat,lon);
    //sat.printf("lat=%f,lon=%f\r\n",lat,lon);
    gps_send(lat,lon);
    
    double lat_sat_deg = lat;
    double long_sat_deg = lon;
    double lat_ht_deg = 35.72348136069703;
    double long_ht_deg = 140.05782349170937;
    double long_sat = toRadian(long_sat_deg);
    double long_ht = toRadian(long_ht_deg);
    double lat_sat = toRadian(lat_sat_deg);
    double lat_ht = toRadian(lat_ht_deg);
    printf("距離=%f\n", distance(long_sat,long_ht,lat_sat,lat_ht));   
    printf("角度=%f", direction(long_sat,long_ht,lat_sat,lat_ht));
}