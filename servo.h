/*
 * @Author Alexander Moeller, Kenneth Schueman, Clayton Reitz, and Nicholas Pinnello.
 */

//Initialize Timer registers for Servo
void servo_init(void);

//Rotate the Servo based on pulse width
int servo_move(float degrees);
