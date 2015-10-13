/*
 * Zach Bowman
 * Anne Pigula
 *
 * 520.448
 * Electronics Design Lab
 *
 * ForgetMeNot
 * 5//13
 */

//*************************************************************
//                    <CODE DESCRIPTION>
//*************************************************************

//-------------------------------------------------------------
//                    <PIN DESCRIPION>
//
// RFID 5V  (PIN 11) --> 5V
// RFID GND (PIN 01) --> GND
// RFID D0  (PIN 09) --> D3
// RFID NC  (PIN 07) --> GND
//
// BUZZER V   (RED)  --> D5
// BUZZER GND (BLK)  --> GND
//
// SWITCH + (RED)    --> 3.3V
// SWITCH - (GRN)    --> 1 kohm --> GND
//      " "          --> D4
//
// HALL EFFECT 1     --> D8
// HALL EFFECT 2     --> GND
// HALL EFFECT 3     --> 5V
//     " "           --> 10 kohm --> HALL EFFECT 1
//                   
//-------------------------------------------------------------
//                      <RFID TAGS>  
// BLACK TAGS: 67005DBE9014     WHITE CARDS: 6A003E7480A0
//             67005DBCCC4A                  6A003E5D737A
//             690025E20FA1                  6A003E301470
//
//-------------------------------------------------------------

#include <WiServer.h>
#include <SoftwareSerial.h>

//GLOBAL
#define WIRELESS_MODE_INFRA	1
#define WIRELESS_MODE_ADHOC	2
#define MAX_USERS               2
#define MAX_OBJECTS             3
 
//Settings
SoftwareSerial id20(3,2); // virtual serial port
const int buzzerPin = 5;
const int switchPin = 4;
const int hallPin = 2;

//Variables 
String recentTag = "";
boolean newRead = false;
boolean newSwitch = false;

//User Information
String user[MAX_USERS] = {"Zach","Anne"}; //user names
String userTags[MAX_USERS][MAX_OBJECTS] = {{"67005DBE9014", "67005DBCCC4A", "690025E20FA1"},
                         {"6A003E7480A0", "6A003E5D737A", "6A003E301470"}} ; //List of RFID TAGS
int userLog[MAX_USERS][MAX_OBJECTS]; //user[x][y] = boolean object scanned
String userObjects[MAX_USERS][MAX_OBJECTS]; // userObjects[x][y]= "Object Name"

void setup() 
{  
 Serial.begin(9600);
 id20.begin(9600);
 pinMode(buzzerPin, OUTPUT);  
 pinMode(switchPin, OUTPUT);
 pinMode(hallPin, INPUT);
 
 delay(2000);
}

//*************************MAIN LOOP***************************
void loop () // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ 
{
  //if door is closed
  if( isDoorOpen() == false )
  {
    if(id20.available()) 
    {
      recentTag = ReadTag();
      buzzer_1();
      newRead = true;
      Serial.println( "Tag Read: " + recentTag );
    } 
    if( newRead )
    {
      markTag( recentTag );
      printMarked();
      newRead = false;
    }
  newSwitch = true;
  //if door is open  
  }else{
    //only if opened once
    if( newSwitch )
    {
      if(printMissing() == true)
      {
      buzzer_2();
      }
      newSwitch = false;
      clearMarked();
    }
  }
}
 // ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 //^^^^^^^^^^^^^^^^^^^^^^^^^MAIN LOOP^^^^^^^^^^^^^^^^^^^^^^^^^^

/*
 * reads and returns rfid tags
 */
String ReadTag()
{
  char x;
  String tag;
  
  while(1)
  {
    if(id20.available()) 
    {
      x = id20.read(); // receive character from ID20
      tag.concat(x);
    
      if (tag.length() == 16)
      { 
        tag = tag.substring(1,13);
        return tag;
      }
    }
  }
}

/*
 * returns 1 if it marks the tag, returns 0 if not marked
 */
int markTag( String tag )
{
   for( int i=0; i<MAX_USERS; i++ )
   {
     for( int j=0; j<MAX_OBJECTS; j++)
     {
       if( tag.compareTo(userTags[i][j]) == 0 && userLog[i][j] != 1 )
       {
         userLog[i][j] = 1;
         return 1; 
       }
     }   
   }
   return 0;
}

/*
 * prints out an int array of users marked/unmarked objects
 */
void printMarked()
{
   for( int i=0; i<MAX_USERS; i++ )
   {
     for( int j=0; j<MAX_OBJECTS; j++)
     {
       Serial.print( userLog[i][j] );
     }
     Serial.println();
   }
}

/*
 * prints out a string of missing objects
 */
boolean printMissing()
{
  boolean isMissing = false;
   for( int i=0; i<MAX_USERS; i++ )
   {
     Serial.print( user[i] + " is missing: " );
     for( int j=0; j<MAX_OBJECTS; j++)
     {  
       if( userLog[i][j] == 0 )
       {
         Serial.print( userTags[i][j] + " " );
         isMissing = true;
       }
     }
     Serial.println();
   }
   //Serial.println(isMissing);
   return isMissing;
}

/*
 * emits a beep sound varient on voltage and timeDelay
 */
void buzzer_1()
{
  double voltage = 3.00;
  int timeDelay = 100; 
  analogWrite(buzzerPin, voltage);   // sets the buzzer on
  delay(timeDelay);                  // waits for .1 second
  analogWrite(buzzerPin, 0);    // sets the buzzer off
}

/*
 * emits a beep sound varient on voltage and timeDelay
 */
void buzzer_2()
{
  double v1 = 7.00;
  double v2 = 12.00;
  double v3 = 17.00;
  
  analogWrite(buzzerPin, v1);     
  delay(100);                    
      analogWrite(buzzerPin, 0); 
      delay(100);      
  analogWrite(buzzerPin, v2);  
  delay(100);      
      analogWrite(buzzerPin, 0);  
      delay(100);      
  analogWrite(buzzerPin, v2);   
  delay(100);       
     analogWrite(buzzerPin, 0);    
     delay(100);     
  analogWrite(buzzerPin, v3);   
  delay(500);       
     analogWrite(buzzerPin, 0); 
     delay(100);
}

/*
 * boolean if switch is on or off
 */
boolean isSwitchOn()
{
   return(digitalRead(switchPin));
}

/*
 * clear all marked tags to 0
 */
void clearMarked()
{
   for( int i=0; i<MAX_USERS; i++ )
   {
     for( int j=0; j<MAX_OBJECTS; j++)
     {
         userLog[i][j] = 0;
     }
   }   
}


/*
 * boolean if door is open or closed
 */
boolean isDoorOpen()
{
  int doorState = digitalRead(hallPin);
  if (doorState == LOW)
  {
    //Serial.println("Door closed");
    return false;
  }
 else
 {
   //Serial.println("Door open");
   return true;
 }
}

