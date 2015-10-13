/*
 * A simple sketch that uses WiServer to serve a web page
 */


#include <WiServer.h>

#define WIRELESS_MODE_INFRA	1
#define WIRELESS_MODE_ADHOC	2
#define MAX_USERS               5
#define MAX_OBJECTS             3

// Wireless configuration parameters ----------------------------------------

// Anne's place, TopFloorNoneighbas
unsigned char local_ip[] = {192,168,0,30};	// IP address of WiShield
unsigned char gateway_ip[] = {192,168,0,1};	// router or gateway IP address
unsigned char subnet_mask[] = {255,255,255,0};	// subnet mask for the local network
const prog_char ssid[] PROGMEM = {"TopFloorNoneighbas"};		// max 32 bytes

unsigned char security_type = 2;	// 0 - open; 1 - WEP; 2 - WPA; 3 - WPA2

// WPA/WPA2 passphrase
const prog_char security_passphrase[] PROGMEM = {"bakerg4y"};	// max 64 characters

// WEP 128-bit keys
// sample HEX keys
prog_uchar wep_keys[] PROGMEM = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,	// Key 0
				  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Key 1
				  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Key 2
				  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00	// Key 3
				};

// setup the wireless mode
// infrastructure - connect to AP
// adhoc - connect to another WiFi device
unsigned char wireless_mode = WIRELESS_MODE_INFRA;

unsigned char ssid_len;
unsigned char security_passphrase_len;
// End of wireless configuration parameters ----------------------------------------

//User profile variables
int user_count = 2;
String user[MAX_USERS] = {"Anne", "Zach"};
//String tag[12] = {"67005DBE9014", "67005DBCCC4A", "690025E20FA1", "6A003E7480A0", "6A003E5D737A", "6A003E301470"};
int userTags[MAX_USERS][MAX_OBJECTS] = { {0, 1, 2},  {3, 4, 5}, 
                                          {-1, -1, -1}, {-1, -1, -1},
                                           {-1, -1, -1} } ;
String userObjects[MAX_USERS][MAX_OBJECTS] = { {"iPhone", "Keys", "Wallet"},
                                                {"Wallet", "Phone", "Keys"}, };

boolean isScanningTags = false;

// This is our page serving function that generates web pages
boolean sendMyPage(char* URL)
{
  if (strcmp(URL, "/") == 0)
  {
    isScanningTags = false;

    // Use WiServer's print and println functions to write out the page content
    WiServer.print(F("<html> <form><b><font size=6>Welcome to ForgetMeNot!</font></b><br><br>"));
    printUsers();
    WiServer.print(F("<br>What would you like to do? <br><br>"));

    WiServer.print(F("<input type='radio' name='task' value='addTags'> Add tags<br>"));
    WiServer.print(F("<input type='radio' name='task' value='assignTags'> Assign tags to users:<br><br>"));
    WiServer.print(F("<input type='radio' name='task' value='addUser'> Add user named:<br>"));
    WiServer.print(F("<input type='radio' name='task' value='removeUser'> Remove user named:<br><br>"));
    WiServer.print(F("<input type='text' name='userChange'><br>"));
    WiServer.print(F("<input type='submit' value='Submit'></form> </html>"));

    return true;
  }

  Serial.print(F("URL is "));
  Serial.println(URL);

  if ( strstr(URL,"addUser") != NULL)
  {
    char newUser[8];
    memcpy(newUser, &URL[26],7);
    newUser[7] = '\0';
    Serial.println(newUser);
    addUser(newUser);
    printUsers();
  }

  if ( strstr(URL,"removeUser") != NULL)
  {
    char exUser[8];
    memcpy(exUser, &URL[29],7);
    exUser[7] = '\0';
    removeUser(exUser);
    printUsers();
    WiServer.print(F("<br><form><input type='submit' value='Go Home'></form>"));
  }
    
  if ( (strstr(URL, "addTags") != NULL) )
  {
      Serial.println(F("Scan tags now"));
      isScanningTags = true;
      WiServer.print(F("<html>Please scan all tags to be added<br><br>"));
      WiServer.print(F("<form><input type='submit' value='Finished scanning'>"));
      WiServer.print(F("</form> </html>"));
      
      // URL was recognized
      return true;    
  }

  if ( (strstr(URL, "assignTags") != NULL) )
  {
    isScanningTags = true;
    WiServer.print(F("<html>Assign tags to users<table>"));
    
    int numTags = 6;
    for (int i = 0; i < numTags; i++)
    {
      WiServer.print(F("<tr><form><td><b>Tag "));
      WiServer.print(i);
      WiServer.print(F("</b></td>"));
      for (int j = 0; j < user_count; j++)
      {
        WiServer.print(F("<td>"));
        WiServer.print(F("<input type='radio' name=tag"));
        WiServer.print(String(i));
        WiServer.print(F(" value="));
        WiServer.print(user[j]);
        for(int k = 0; k < MAX_OBJECTS; k++)
        {
          if (userTags[j][k] == i)
            WiServer.print(F(" checked"));
        }
        WiServer.print(F(">"));
        WiServer.print(user[j]);
        WiServer.print(F("<br>"));
        WiServer.print(F("</td>"));
      }      
      WiServer.print(F("</tr>"));    
    }
    
    WiServer.print(F("</table><form><input type='submit' value='Finished assigning'>"));
    WiServer.print(F("</form> </html>"));
    
    return true;    
  }

  if ( strstr(URL,"tag") != NULL)
  {
    assignTags(URL);
    WiServer.print(F("<br><form><input type='submit' value='Go Home'></form>"));
  }

  return false;
}

void setup()
{
  // Initialize WiServer and have it use the sendMyPage function to serve pages
  WiServer.init(sendMyPage);
  
  // Enable Serial output and ask WiServer to generate log messages (optional)
  Serial.begin(57600);
  WiServer.enableVerboseMode(true);
  
}

void loop()
{
  // Run WiServer
  WiServer.server_task();
  
  delay(10);
}

/*
 *  Returns index of inputUser in user[], -1 if not found
 */
int findUser( String inputUser )
{
  Serial.println(inputUser);
  for( int i = 0; i < user_count; i++)
  {
    if( inputUser == user[i] )
    {
      return i; 
    }
  }
  return -1;
}//end of findUser()

void addUser( char* newUser )
{
  if (findUser(newUser) == -1)
  {
    user[user_count] = newUser;
    user_count = user_count+1;

    Serial.print(F("user_count is: "));
    Serial.println(user_count);
    Serial.print(F("user_name is: "));
    Serial.println(user[user_count]);
  }

  delay(100);
}//end of addUser()

void removeUser(char* exUser)
{
  int index = findUser( exUser );
  if ( index == -1)
  {
    Serial.println(F("ERROR: USER NOT FOUND"));
    return; 
  }
  else
  {
    for( int i = index; i < user_count-1; i++)
      {
        for ( int h = 0; h < MAX_OBJECTS; h ++ )
        {
          userTags[i][h] = userTags[i+1][h];
          userTags[user_count-1][h] = -1;
        }
        user[i] = user[i+1];
      }
  }
  user[user_count-1] = "";

  Serial.print(F("Removed user: "));
  Serial.println(exUser);

  user_count = user_count-1; 
  printUsers();
  delay(100);
}//end of removeUser

void assignTags(String url)
{
  clearUserTags();
  Serial.println(url);
  for (int b = 0; b < 6; b++)
  {
    int start = url.indexOf(String(b)) + 2;
    String owner = url.substring(start, start + 10);
    owner = owner.substring(0, owner.indexOf("&"));
    assignOneTag(owner, b);
  }
  Serial.println(F("End assignTags"));
}//end of assignTags

void assignOneTag(String owner, int tagNumber)
{
  Serial.println(F("assignOneTag() called"));
  Serial.println(owner);
  int ownerIndex = findUser(owner);
  
  for (int c = 0; c < MAX_OBJECTS; c++)
  {
    if (userTags[ownerIndex][c] == -1)
    {
      userTags[ownerIndex][c] = tagNumber;
      Serial.println(F("Tag assigned"));
      Serial.println(userTags[ownerIndex][c]);
      break;
    }
  }
  printUsers();
}

void clearUserTags()
{
  Serial.println(F("clearUserTags() called"));
  for (int c = 0; c < MAX_USERS; c++)
  {
    for (int d = 0; d < MAX_OBJECTS; d++)
    {
      userTags[c][d] = -1;
    }
  }
  printUsers();
}//end of clearUserTags

void printUsers()
{
  WiServer.print(F("<u>Current system settings</u><br>"));
  for (int c = 0; c < user_count; c++)
  {
    WiServer.print(user[c] + " has tags:   ");
    for (int d = 0; d < MAX_OBJECTS; d++)
    {
      if ( userTags[c][d] != -1)
      {
        if ( d != 0)
        {
          WiServer.print(F(", "));
        }
        
        WiServer.print(userTags[c][d]);
        WiServer.print(F(" for "));
        WiServer.print(userObjects[c][d]);
      }
    }
    WiServer.print(F("<br>"));
  }
}//end of printUsers
/*
void printUsers()
{
  WiServer.print(F("Current system settings<br>"));
  for (int c = 0; c < user_count; c++)
  {
    WiServer.print(user[c] + " has tags ");
    for (int d = 0; d < MAX_OBJECTS; d++)
    {
      if ( userTags[c][d] != -1)
      {
        WiServer.print(userTags[c][d]);
        WiServer.print(F(" "));
      }
    }
    WiServer.print(F("<br>"));
  }
}//end of printUsers*/
