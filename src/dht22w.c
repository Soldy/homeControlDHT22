/*
 *  dht22.c:
 * read temperature and humidity from DHT22 sensor and save it to log and show on screen
 */
 
#include <wiringPi.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
 
#define MAX_TIMINGS 85
#define DHT_PIN 0

/* 
* 3 = GPIO-22 
*/
 
FILE *fp;
int data[5] = { 0, 0, 0, 0, 0 };
int packages = 0;
int errors = 0;
int tenround = 0;
float tempetureout = 0;
float humidityout = 0;
int resultstatus = 0;

void clearScreen(void){
    printf("\033[2J");
    printf("\033[H");
}

void tableMake(void){
    clearScreen();
    printf("\033[2;2H ------------------ ");
    printf("\033[3;6H  Hunidity ");
    printf("\033[4;5H  Tempeture ");
    printf("\033[5;8H  Result ");
    printf("\033[6;6H  Packages ");
    printf("\033[7;8H  Errors ");
    printf("\033[8;2H ------------------ ");
    printf("\033[3;28H%%");
    printf("\033[4;28HC");
}

void tableResult(){
    printf("\033[3;20H %.2f", humidityout);
    printf("\033[4;20H %.2f", tempetureout);
    printf("\033[5;20H %d ", resultstatus);
    printf("\033[6;20H %d ", packages);
    printf("\033[7;20H %d ", errors);
    printf("\033[7;26H - %.2f %% ", (((float)(errors)/(float)(packages))*100));
}

void read_dht_data(){
    uint8_t laststate = HIGH;
    uint8_t nextstate = HIGH;
    uint8_t counter  = 0;
    uint8_t j   = 0, i;
    data[0] = data[1] = data[2] = data[3] = data[4] = 0; 
    /* pull pin down for 16 milliseconds */
    pinMode( DHT_PIN, OUTPUT );
    digitalWrite( DHT_PIN, LOW );
    delay( 16 );//
    /* prepare to read the pin */
    pinMode( DHT_PIN, INPUT );
    /* detect change and read data */
    for ( i = 0; i < MAX_TIMINGS; i++ ){
         counter = 0;
         while ( nextstate == laststate ){
             nextstate = digitalRead( DHT_PIN );
             counter++;
            // usleep( 1900 );
             delayMicroseconds( 2 );
             if ( counter == 255 )
                 break;
         }
         laststate = nextstate;
         if ( counter == 255 )
             break;
         /* ignore first 3 transitions */
         if ( (i >= 4) && (i % 2 == 0) ){
             /* shove each bit into the storage bytes */
             data[j / 8] <<= 1;
             if ( counter > 16 )
                 data[j / 8] |= 1;
             j++;
         }
    }
    /*
     * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
     * print it out if data is good
     */
     humidityout = (float)((data[0] << 8) + data[1]) / 10;
     tempetureout = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
     resultstatus = 0;
     packages++;
     if ( data[2] & 0x80 )
         tempetureout = -tempetureout;
//     float f = c * 1.8f + 32;
     if ( (j != 40) &&(data[4] != ( (data[0] + data[1] + data[2] + data[3]) & 0xFF) ) ){
         resultstatus = 3;
         errors++;
     } else if (40 != j){
         resultstatus = 2;
         errors++;
     } else if (data[4] != ( (data[0] + data[1] + data[2] + data[3]) & 0xFF) ){
         resultstatus = 1;
         errors++;
     }
}

int main( void ){
    if ( wiringPiSetup() == -1 )
        exit( 1 );
    while ( 1 ){
         if (tenround > 9){
             tenround=0;
             tableMake();
         }
         read_dht_data();
         tableResult();
         fflush(stdout);
         usleep( 2000000 ); /* wait 2 seconds before next read */
         tenround++;
    }
    return quit();
}

