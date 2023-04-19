#include "lcd.h"
#include <math.h>
#include "servo.h"
#include "ping.h"
#include "adc.h"
#include "open_interface.h"
#include "Timer.h"
#include "uart_extra_help.h"
#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

extern volatile enum {LOW, HIGH, DONE} state;  //Set by ISR
extern volatile unsigned int rising_time;  //Pulse start time
extern volatile unsigned int falling_time; //Pulse end time
extern volatile char uart_data;
extern volatile char flag;
extern volatile int deg;
extern volatile int direction;
extern volatile float pcv;
extern volatile int right_calibration_val = 313200;
extern volatile int left_calibration_val = 287172;

void turn_counterclockwise(oi_t *sensor, short degrees);
void turn_clockwimse(oi_t *sensor, short degrees);
int oneEightyScan();
void move_backwards(oi_t *sensor);
void move_forward(oi_t *sensor, float millimeters);

/*
 * @Author Alexander Moeller, Kenneth Schueman, Clayton Reitz, and Nicholas Pinnello.
 */

//Stores all of the information about each object scanned
typedef struct {
    short object;
    short start_angle;
    short end_angle;
    short mid_angle;
    double ping_distance;
    short adc_distance;
    double linear_width;
    char type;
} objects;

int main(void) {
    //Initializations
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    timer_init();
    lcd_init();
    adc_init();
    uart_init(115200);
    uart_interrupt_init();
    servo_init();
    ping_init();

    while (1) {
        oi_update(sensor_data);

        //Conditions to override manual control: hole, boundary, or small object
        //If there is a hole, then avoid falling into it
        if (sensor_data->cliffFrontLeftSignal < 20 ||  sensor_data->cliffFrontRightSignal < 20 ||  sensor_data->cliffLeftSignal < 20 || sensor_data->cliffRightSignal < 20) {
            uart_sendStr("\n\rOVERRIDE: Hole in the way!");
            oi_setWheels(0, 0);

            //If hole is on left side of the bot, turn right
           if (sensor_data->cliffLeftSignal < sensor_data->cliffRightSignal) {
               move_backwards(sensor_data);
               turn_clockwise(sensor_data, 90);
           }

           //Otherwise, turn left
           else {
               move_backwards(sensor_data);
               turn_counterclockwise(sensor_data, 90);
           }
        }

        //If there is a boundary, move backwards and stay in the course
        if (sensor_data->cliffFrontLeftSignal > 2700 || sensor_data->cliffFrontRightSignal > 2600) {
            uart_sendStr("\n\rOVERRIDE: Hit the boundary!");

           if (sensor_data->cliffLeftSignal < sensor_data->cliffRightSignal) {
               move_backwards(sensor_data);
               turn_clockwise(sensor_data, 90);
           }
           else {
               move_backwards(sensor_data);
               turn_counterclockwise(sensor_data, 90);
           }
        }

        //If the bot bumps into a small object on the left, move around it to the right
        if (sensor_data->bumpLeft) {
            uart_sendStr("\n\rOVERRIDE: Bumped into object on the left side!");
            move_backwards(sensor_data);
            oi_update(sensor_data);
            turn_clockwise(sensor_data, -80);
        }

        //If the bot bumps into a small object on the right, move around it to the left
        else if (sensor_data->bumpRight) {
            uart_sendStr("\n\rOVERRIDE: Bumped into object on the right side!");
            move_backwards(sensor_data);
            oi_update(sensor_data);
            turn_counterclockwise(sensor_data, 80);
        }

        //If a key was pressed in Putty, respond accordingly
        if (flag) {
            //Send character received to the Putty
            //uart_sendChar(uart_data);

            //If the character is 'w', drive forward
            if (uart_data == 'w') {
                oi_setWheels(100, 100);
            }

            //If the character is 's', drive backwards
            else if (uart_data == 's') {
                oi_setWheels(-100, -100);
            }

            //If the character is 'a', turn left
            else if (uart_data == 'a') {
                oi_setWheels(50, -50);
            }

            //If the character is 'd', turn right
            else if (uart_data == 'd') {
                oi_setWheels(-50, 50);
            }

            //If the character is 'm', stop the bot and do a 180 degree scan
            else if (uart_data == 'm') {
                oi_setWheels(0, 0);
                int humans = oneEightyScan();

                //If humans is 1, a survivor is found, if 2, first responders were found
                if (humans) {
                    lcd_printf("HUMAN FOUND!");
                    lcd_home();
                    uart_sendStr("\n\rLocated survivor, find first responders to report!");
                }
                else if (humans == 2) {
                    lcd_printf("Located first responders.");
                    lcd_home();
                    uart_sendStr("\n\rLocated survivor, find first responders to report!");
                }
            }

            //If any other key is pressed, stop the bot and do nothing
            else {
                oi_setWheels(0, 0);
            }

            flag = 0;
        }
    }

	return 0;
}

void turn_clockwise(oi_t *sensor, short degrees) {
    double sumTurnRight = 0.0;
    oi_setWheels(-50, 50);

    while ((short) sumTurnRight > degrees) {
        oi_update(sensor);
        sumTurnRight += sensor->angle;
    }

    oi_setWheels(0, 0);
}

void turn_counterclockwise(oi_t *sensor, short degrees) {
    double sumTurnLeft = 0;
    oi_setWheels(50, -50);

    while ((short) sumTurnLeft < degrees) {
        oi_update(sensor);
        sumTurnLeft += sensor->angle;
    }

    oi_setWheels(0, 0);
}

void move_forward(oi_t *sensor, float millimeters) {
    double sum = 0.0;
    oi_setWheels(150, 150);

    //Move given distance (used in override for hole, boundary, and small object)
    while (sum < millimeters) {
       oi_update(sensor);
       sum += sensor->distance;
    }

    //Stop after moving the specified distance
    oi_setWheels(0, 0);
}

void move_backwards(oi_t *sensor) {
    double sumBack = 0.0;

    //Move backwards 150mm
    oi_setWheels(-100,-100);
    while (sumBack > -75) {
        oi_update(sensor);
        sumBack += sensor->distance;
    }
}

int oneEightyScan() {
    int i;
    int humanType = 0; //0 is no humans, 1 is stranded human, 2 is first responder group
    int humanCount = 0;

    //Values for displaying data
    char start[] = "Degrees\t\tDistance (cm)\n\r";
    char end[] = "Object#\t\tAngle\t\tPing Distance\tADC Distance\tWidth\tType\n\r";

    uart_sendStr("\n\r");
    uart_sendStr(start);

    //Values to store data collected from the scan
    char layout[100];
    short objectCount = 0;
    objects objectArr[10];

    //Values to note whether and object is being scanned
    int previousVal = 0;
    int previousAngle = 0;
    int curVal;
    int distVal = 70;

    //Scan loop for 180 degrees
    for (i = 0; i <= 180; i += 2) {
        servo_move(i);

        //Give bot time to turn
        if (i == 0) {
            timer_waitMillis(500);
        }

        //Store current IR distance in cm
        curVal = 654371 * pow(adc_read(), -1.420);

        //If an object is scanned, store its values
        if (curVal < distVal && previousVal > distVal) {
            objectArr[objectCount].object = objectCount + 1;
            objectArr[objectCount].start_angle = i;
            objectArr[objectCount].adc_distance = curVal;
            objectCount++;
        }

        //If object is still being scanned, continue to update width
        else if (previousVal < distVal) {
            objectArr[objectCount - 1].end_angle = previousAngle;
        }

        sprintf(layout, "%d\t\t%d\n\r", i, curVal);
        uart_sendStr(layout);
        previousVal = curVal;
        previousAngle = i;
    }

    //Find the middle angle, then turn to it and scan the ping distance
    for (i = 0; i < objectCount; i++) {
        objectArr[i].mid_angle = (objectArr[i].start_angle + objectArr[i].end_angle) / 2;
        servo_move(objectArr[i].mid_angle);
        timer_waitMillis(500); //Give scanner time to move over
        objectArr[i].ping_distance = ping_read();

        //Calculate the linear width with trig
        objectArr[i].linear_width = objectArr[i].ping_distance * sin((objectArr[i].end_angle - objectArr[i].start_angle) * (3.14 / 180.0));

        //Figure out what type of object was scanned (H is human, T is tree, D is large debris)
        if (objectArr[i].linear_width <= 11.0 && objectArr[i].linear_width > 6.0) {
            objectArr[i].type = 'H';
            humanCount++;
        }
        else if (objectArr[i].linear_width <= 14.0 && objectArr[i].linear_width > 11.0) {
            objectArr[i].type = 'T';
        }
        else if (objectArr[i].linear_width > 14.0 || objectArr[i].linear_width < 6.0) {
            objectArr[i].type = 'D';
        }
    }

    //Figure out if it is a stranded survivor or the first responders
    if (humanCount >= 3) {
        humanType = 2;  //First responder group
    }
    else if (humanCount < 3 && humanCount > 0) {
        humanType = 1;  //Stranded survivor
    }

    //Formatting to print out the objects and their values to Putty
    uart_sendStr("\n\r");
    uart_sendStr(end);
    for (i = 0; i < objectCount; i++) {
        sprintf(layout, "%d\t\t%d\t\t%0.2f\t\t%d\t\t%0.2f\t%c\n\r", objectArr[i].object, objectArr[i].mid_angle, objectArr[i].ping_distance, objectArr[i].adc_distance, objectArr[i].linear_width, objectArr[i].type);
        uart_sendStr(layout);
    }

    return humanType;
}
