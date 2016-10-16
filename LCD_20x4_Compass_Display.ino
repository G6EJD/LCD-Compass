/*
3-Axis Compass Module (HMC5883) Demo code.

Usage:
        1. Run as is - it prints out detailed info on heading
        2.  To calibrate YOUR sensor for OFFSET - XY scatter plot method:-
                - uncomment "//#define RAW_DATA_CSV" to get data for calibration in CSV format.
                - load sketch to your arduino
                - while running the sketch, with the serial terminal open, rotate your arduino through a FULL 360 degrees (and a bit extra!)
                - Copy ALL data from the serial terminal and do an XY scatter plot in your favour tool (Libre Office Calc etc)
                    - you may find it easiest to press & hhold the Arduino reset button, then press ctl A, ctl C to copy the data.
                    - you may also need to tidy the start & end of the data for stray characters!
        3.  To calibrate YOUR sensor for OFFSET - using the processing XY scatter plot method:-
                - uncomment "//#define RAW_DATA_CSV" to get data for calibration in CSV format.
                - Also uncomment "//#define RAW_DATA_ARRAY" to get data for calibration in CSV format.
                - load sketch to your arduino
                - while running the sketch, with the serial terminal open, rotate your arduino through a FULL 360 degrees (and a bit extra!)
                - Copy ALL data from the serial terminal into your processing sketch. My example code is at usabledevices.com
                    - you may find it easiest to press & hhold the Arduino reset button, then press ctl A, ctl C to copy the data.
                    - you may also need to tidy the start & end of the data for stray characters!
        4.  Then work out the offset values for x and y and add below.
        5.  If the xy plot is NOT circular then:-
            - uncomment "//#define SCALE" and adjust the scale values until your XY scatter plots become circular.
*/
 
/* Background information on the HMC5883 compass sensor & configuration / calibration
Here are most of the sources I used for these updates - apologies if I forgot anyone :(, please let me know - Spanner888 at usabledevices.com

// Calculate heading when the magnetometer is level, then correct for signs of axis.
float heading = atan2(raw.YAxis - y_offset, raw.XAxis - x_offset);
with x_offset = (x_max_value_measured - x_min_value_measured)/2
same with y_offset
You can get the max & min value of each channel (x, y, z) by rotating the HMC5883L board over all axis.
 
it is important to calibrate the magnetormeters on the site where you use it.
 
The process:
* Attach your magnetometer to its final place (your vehicle may have a constant magnetic field)
* Do some turns with your vehicle (we have done it in 2D, but 3D patterns may also work)
* Draw the measured points. They should align with a circle.
* Find the center of the circle. Save the coordinates.
* When measuring the heading you have to count with that the heading is relative to the middle that you have previously measured!
So substract the center coordinates from what you have measured before counting atan2 to find the heading angle.

See data sheet AN203_Compass_Heading_Using_Magnetometers.pdf
*/

#include <LiquidCrystal_I2C.h>
// LCD Screen Line1 is 254, 128
// LCD Screen Line2 is 254, 192
// LCD Screen Line3 is 254, 148
// LCD Screen Line4 is 254, 212

LiquidCrystal_I2C lcd(0x27,20,4); //set the LCD address to 0x27 for a 20 chars and 4 line display
#include <Wire.h>
#define I2C_TX write
#define I2C_RX read

//#define SCALE            //use this to scale each axis if the test data does not give a circular (spherical 3D) plot
                           // done as #def for max performance of code not using the scale factors!
 
 // use to switch the serial output data to a CSV format of raw x,y,z data values to help calibrate your compass
#define RAW_DATA_CSV
// ALSO use this #def to format output to easly paste into a data array - eg to display data in Processing. See usabledevices.com for example.
#define RAW_DATA_ARRAY
 
// Shift the device's documented slave address (0x3C) for write operation
// 1 bit right.This compensates for how the TWI library only wants the
// 7 most significant bits (with the high bit padded with 0)
 
#define HMC5883_WriteAddress 0x1E //  i.e 0x3C >> 1
#define HMC5883_ModeRegisterAddress 0x02
#define HMC5883_ContinuousModeCommand (uint8_t)0x00     // cast to uint8_t added to get code to compile under Arduino v1.0
#define HMC5883_DataOutputXMSBAddress 0x03
 
int regb=0x01;
int regbdata=0x40;
int outputData[6];
 
// calibrate YOUR compass, so compass raw x,y (,z) data is centered around 0,0,0 axis!
// To do this read do an XY scatter plot for 2D xy data, and just "read"
//   work out or guess the offset to center the data (you need to rotate the compass to get lots of data pairs)
//   for my compass the values where x=-100, y=-100 (did not work out z axis!)
const int x_offset = 0;
const int y_offset = 0;
const int z_offset = 0;
 
#ifdef SCALE
const int x_scale = 1;
const int y_scale = 1;
const int z_scale = 1;
#endif
 
void setup()
{
  lcd.init();
  lcd.begin(20,4);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.clear();
  Serial.begin(9600);
  Wire.begin();   //Initiate the Wire library and join the I2C bus as a master
  delay(2000);    // wait for disaplay to start
  Serial.write(254);
  Serial.write(1);
  delay(500); // wait for display to clear 
}
 
void loop() {
  int i,x,y,z;
  double angle;
  lcd.clear();
  delay(500); // Wait for the screen to clear 
  Serial.write(254);
  Serial.write(128); //Goto line-1
  Wire.beginTransmission(HMC5883_WriteAddress);
  Wire.I2C_TX(regb);
  Wire.I2C_TX(regbdata);
  Wire.endTransmission();
 
  delay(1000);
  Wire.beginTransmission(HMC5883_WriteAddress); //Initiate a transmission with HMC5883 (Write address).
  Wire.I2C_TX(HMC5883_ModeRegisterAddress);     //Place the Mode Register Address in send-buffer.
  Wire.I2C_TX(HMC5883_ContinuousModeCommand);   //Place the command for Continuous operation Mode in send-buffer.
  Wire.endTransmission();                       //Send the send-buffer to HMC5883 and end the I2C transmission.
  delay(100);
 
  Wire.beginTransmission(HMC5883_WriteAddress);  //Initiate a transmission with HMC5883 (Write address).
  Wire.requestFrom(HMC5883_WriteAddress,6);      //Request 6 bytes of data from the address specified.
 
  delay(500);
 
  //Read the value of magnetic components X,Y and Z
 
  if(6 <= Wire.available()) // If the number of bytes available for reading be <=6.
  {
    for(i=0;i<6;i++)
    {
      outputData[i]=Wire.I2C_RX();  //Store the data in outputData buffer
    }
  }
 
  x=outputData[0] << 8 | outputData[1]; //Combine MSB and LSB of X Data output register
  z=outputData[2] << 8 | outputData[3]; //Combine MSB and LSB of Z Data output register
  y=outputData[4] << 8 | outputData[5]; //Combine MSB and LSB of Y Data output register
  
#ifdef SCALE
   angle= atan2((double)y * y_scale - y_offset,(double)x * x_scale - x_offset)* (180 / 3.14159265) +180; // angle in degrees
#else
   angle= atan2((double)y - y_offset,(double)x - x_offset)* (180 / 3.14159265) +180; // angle in degrees
#endif

  /*
   Refer the following application note for heading calculation.
   http://www.ssec.honeywell.com/magnetic/datasheets/lowcost.pdf
   ----------------------------------------------------------------------------------------
   atan2(y, x) is the angle in radians between the positive x-axis of a plane and the point
   given by the coordinates (x, y) on it.
   ----------------------------------------------------------------------------------------
   This sketch does not utilize the magnetic component Z as tilt compensation can not be done without an Accelerometer
    ----------------->y
   |
   \/
   x
  
         N
   NW    |    NE
         |
   W----------E
         |
   SW    |    SE
         S
 
   */
// only print the detail if NOT doing either sending either calibration data set to serial
// #ifndef RAW_DATA_CSV
// #ifndef RAW_DATA_ARRAY

//Print the anglke on the LCD Screen
  Serial.write(254);
  Serial.write(128); //Goto line 1
  lcd.print("Angle=");
  lcd.print(angle,2);

  Serial.write(254);
  Serial.write(192); //Goto line 2
  
  //Print the approximate direction
  lcd.print("Heading: ");
  if((angle < 22.5) || (angle > 337.5 ))
    lcd.print("South");
  if((angle > 22.5) && (angle < 67.5 ))
    lcd.print("South-West");
  if((angle > 67.5) && (angle < 112.5 ))
    lcd.print("West");
  if((angle > 112.5) && (angle < 157.5 ))
    lcd.print("North-West");
  if((angle > 157.5) && (angle < 202.5 ))
    lcd.print("North");
  if((angle > 202.5) && (angle < 247.5 ))
    lcd.print("NorthEast");
  if((angle > 247.5) && (angle < 292.5 ))
    lcd.print("East");
  if((angle > 292.5) && (angle < 337.5 ))
    lcd.print("SouthEast");

  //Print the approximate direction
  Serial.print("You are heading ");
  if((angle < 22.5) || (angle > 337.5 ))
    Serial.print("South");
  if((angle > 22.5) && (angle < 67.5 ))
    Serial.print("South-West");
  if((angle > 67.5) && (angle < 112.5 ))
    Serial.print("West");
  if((angle > 112.5) && (angle < 157.5 ))
    Serial.print("North-West");
  if((angle > 157.5) && (angle < 202.5 ))
    Serial.print("North");
  if((angle > 202.5) && (angle < 247.5 ))
    Serial.print("NorthEast");
  if((angle > 247.5) && (angle < 292.5 ))
    Serial.print("East");
  if((angle > 292.5) && (angle < 337.5 ))
    Serial.print("SouthEast");
 
  Serial.print(": Angle between X-axis and the South direction ");
  if((0 < angle) && (angle < 180) )
  {
    angle=angle;
  }
  else
  {
    angle=360-angle;
  }
  Serial.print(angle,2);
  Serial.println(" Deg");
//#endif
//#endif
 
 
#ifndef RAW_DATA_ARRAY
	Serial.print("{ ");
#endif
 
#ifndef RAW_DATA_CSV
	Serial.print(angle);
	Serial.print(", ");
	Serial.print(x);
	Serial.print(", ");
	Serial.print(y);
	Serial.print(", ");
	Serial.println(z);
#endif
 
#ifndef RAW_DATA_ARRAY
	Serial.print(" }, ");
#endif
 
  delay(100);
}
