  /*
 Book In Pocket  TFT
*/

#include "strings_rus.h"
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>

#include "font7_12px_fixed.h";
TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

void setMyFont(){
  tft.setFreeFont(&exportFont);
}

float p = 3.1415926;
int key=34;
int bkl=32;
TFT_eSprite stext1= TFT_eSprite(&tft); // Sprite object stext1 ;
uint8_t menu_redraw_required = 0;
#include "SdFat.h"
#include <Arduino.h>
// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 1
/*
  Change the value of SD_CS_PIN if you are using SPI and
  your hardware does not use the default value, SS.
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/
#define SDCARD_SS_PIN 4
// SDCARD_SS_PIN is defined for the built-in SD on some boards.
//#ifndef SDCARD_SS_PIN
//const uint8_t SD_CS_PIN = SS;
//#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
//#endif  // SDCARD_SS_PIN

// Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SPI_CLOCK SD_SCK_MHZ(25)

// Try to select the best SD card configuration.
/*#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif  ENABLE_DEDICATED_SPI*/
//#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SPI_CLOCK)
//#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)
//#endif  // HAS_SDIO_CLASS

#if SD_FAT_TYPE == 0
SdFat sd;
File dir;
File file;
File fileB;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 dir;
File32 file;
File32 fileB;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile dir;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile dir;
FsFile file;
FsFile fileB;
#else  // SD_FAT_TYPE
#error invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE
//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(&Serial, F(s))
//------------------------------------------------------------------------------

#define tftCS 5

#include <WiFi.h>
void setup(void) {
 
   pinMode(tftCS, OUTPUT);
      pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(tftCS, HIGH);
   digitalWrite(SD_CS_PIN, HIGH);
  Serial.begin(115200);
setCpuFrequencyMhz(80); //Set CPU clock to 80MHz fo example
  Serial.println();
  Serial.print("CPU Frequency is: ");
  Serial.print(getCpuFrequencyMhz()); //Get CPU clock
  Serial.print(" Mhz");
  Serial.println();
  WiFi.mode(WIFI_OFF);
  btStop();
  
  pinMode(key,INPUT);
  //pinMode(bkl,OUTPUT);
     //digitalWrite(bkl, LOW);
     ledcAttachPin(bkl,0);  
     ledcSetup(0, 44100, 8); //44100
     ledcWrite(0, 255);
  // Wait for USB Serial
  while (!Serial) {
    yield();
  }
  
  /*Serial.println("Type any character to start");
  while (!Serial.available()) {
    yield();
  }*/
  
//digitalWrite(SD_CS_PIN, LOW);
 //tft.begin(); 
 initMenu();
   
    menu_redraw_required = 1;     // force initial redraw
    
  // digitalWrite(SD_CS_PIN, HIGH);  
  Serial.print("Hello! ST7735 TFT Test");
 //digitalWrite(tftCS, LOW);
 // // Use this initializer if you're using a 1.8" TFT
  tft.init();   // initialize a ST7735S chip
//tft.setRotation(1);
  Serial.println("Initialized");

  uint16_t time = millis();
  tft.fillScreen(TFT_BLACK);
  time = millis() - time;
  
  //stext1= &TFT_eSprite(&tft); // Sprite object stext1
  
  //stext1.setColorDepth(4);
   stext1.setTextWrap(false);
     stext1.setFreeFont(&exportFont); 
  stext1.createSprite(128,128);
  //stext1.fillSprite(TFT_BLUE); // Fill sprite with blue
  stext1.fillSprite(TFT_BLACK); // Fill sprite with blue
  stext1.setScrollRect(0, 0, 128,128, TFT_BLACK);     // here we set scroll gap fill color to blue
  stext1.setTextColor(TFT_WHITE); // White text, no background
  stext1.setTextDatum(BC_DATUM);  // Bottom right coordinate datum
//digitalWrite(tftCS, HIGH);

  //digitalWrite(tftCS, LOW);
  Serial.println(time, DEC);
  //delay(500);

  // large block of text
  tft.fillScreen(TFT_BLACK);
  
   

  
}

int textReadMode=0;//0-scroll, 1-one wordspeed read, 2-manual scroll
int textSize=0;//0-small, 1-big, 2-wide

const int chipSelect = 4;
String line1;
String line2;
String line3;
String line4;
String line5;
String line6;
bool first=true;
int wordReaded=0;
long totalWords=10000;

String bookName; 
int counter = 0;  
String encdir ="";
#define sw  7

int MENU_ITEMS= 0;
int max_menu_items = 9;
int start_menu_items=0;

char *menu_strings[100]; 
int speed=250;
 
uint8_t menu_current = 0;


int fsmState=0;//0 - main menu ; 1- book read mode, 2-book read menu
bool pauseBook=false;
String prev_key_id;

bool sdError=false;

String getButton()
{
  int i, z, sum;
  String button;

  sum = 0;
  for (i=0; i < 4; i++)
  {
     sum += analogRead(key);
  }
  z = sum / 4;
  if (z > 4000) button = "NONE";    
  else if(abs(z-0)<10)button="LEFT";
  else if(abs(z-400)<300)button="UP";
  else if(abs(z-1233)<300)button="DOWN";
  else if(abs(z-1865)<300)button="RIGHT";
  else if(abs(z-2736)<300)button="ENTER";                                        
  else button = "NONE";     

  return button;
}
bool hasBookmark=false;
int wordsSkip=1;
String last_pressed_button;
bool nextPageGo=false;
unsigned long time1;

 #define MFILE_WRITE (O_RDWR | O_CREAT | O_AT_END)
  #define MFILE_READ (O_RDONLY )
char cdir[80] ="/";
void saveBookmark()
{
  Serial.printf("saveBookmark");
    char buffer[380];  
    sprintf(buffer, "%s%s;%6d",cdir,menu_strings[menu_current],wordReaded);
  Serial.printf(buffer);
  int p=  getBookmark();
  Serial.printf("bookmark: %d\n",p);
  if(p==-1)
  {    
    fileB = sd.open("arduino_bookmarks.txt", MFILE_WRITE); 
        fileB.println(buffer);
        Serial.printf("puts line");
        fileB.close();
        hasBookmark=true;
   } else {
    replaceBookmark();
   }     
}

const size_t LINE_DIM = 550;
char line[LINE_DIM];

int getBookmark()
{
  Serial.println("getBookmark");
    char buffer[280];  
    sprintf(buffer, "%s%s",cdir,menu_strings[menu_current]);
    fileB = sd.open("arduino_bookmarks.txt", MFILE_READ);
  size_t n;
  
  if (fileB) 
  {
        while ((n = fileB.fgets(line, sizeof(line))) > 0) {
            char * pch;
            pch = strtok(line,";");
            int cntr=0;            
            Serial. printf ("%s\n",pch);
            if(!strcmp(pch,buffer))
      {
        pch = strtok(NULL, ";");
                int wordToGo = atoi(pch);
                Serial.printf ("%s\n",pch);
                Serial.printf ("word to go: %d\n",wordToGo);
                return wordToGo;
            }
            cntr++;
      }
    fileB.close();
  }
  else
  Serial.println("arduino_bookmarks.txt not found");
  return -1;  
}

int replaceBookmark()
{
  Serial.println("replaceBookmark");
    char buffer[280];  
     sprintf(buffer, "%s%s",cdir,menu_strings[menu_current]);
      fileB = sd.open("arduino_bookmarks.txt", O_RDWR);
  size_t n;
  // if the file opened okay, write to it:
  if (fileB) {
        while ((n = fileB.fgets(line, sizeof(line))) > 0) {
            char * pch;
            pch = strtok(line,";");
            int cntr=0;
            //while (pch != NULL)
            {
               Serial. printf ("%s\n",pch);
             //   pch = strtok(NULL, ";");
                   if(!strcmp(pch,buffer)){
                    char rep[6];
                    sprintf(rep,"%6d",wordReaded);
                    fileB.seek(fileB.position()-n+strlen(pch));//6+newline
                    fileB.write(rep);
                     break;
                   }
                 cntr++;
            }
      }
    fileB.close();
  }else
  Serial.println("arduino_bookmarks.txt not found");
  return -1;
}

void loadBookmark()
{
  Serial.println("loadBookmark");
    char buffer[280];  
     sprintf(buffer, "%s%s",cdir,menu_strings[menu_current]);
      fileB = sd.open("arduino_bookmarks.txt", MFILE_READ);
  size_t n;
  // if the file opened okay, write to it:
  if (fileB) {
        while ((n = fileB.fgets(line, sizeof(line))) > 0) {
            Serial. println (line);
            char * pch;
            pch = strtok(line,";");
            int cntr=0;
            //while (pch != NULL)
            {
               Serial. printf ("%s\n",pch);
                //pch = strtok(NULL, ";");
                   if(!strcmp(pch,buffer)){
                  pch = strtok(NULL, ";");
               int wordToGo=   atoi(pch);
                   Serial.printf ("%s\n",pch);
                     Serial.printf ("word to go: %d\n",wordToGo);
                        file.seek(0);
                             wordReaded=0;
                             wordsSkip=wordToGo;
                     break;
                   }
                 cntr++;
            }             
      }
    fileB.close();
  }else
  Serial.println("arduino_bookmarks.txt not found");
  
}
bool line1Changed=false;
bool spriteClearRequired=false;
void setupBook()
{
    setMyFont();
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_YELLOW);
    int padding = tft.textWidth(LOADING_STRING, 1); // get the width of the text in pixels
    padding/=2;
    tft.setCursor(64-padding,64);
    tft.print(LOADING_STRING);
 
  wordReaded=0;
  pauseBook=false;
  int n;
  char buf[1];
  totalWords=0;
     
  char buffer[80];
    
  sprintf(buffer, "%s%s",cdir,menu_strings[menu_current]);  
     
  bookName=menu_strings[menu_current];
  
  Serial.print("opening book: ");
  Serial.println(buffer);
  char buf2[16];
  file = sd.open(buffer);
    Serial.println(file);
  if(file.isDir() || !strcmp(menu_strings[menu_current],".."))
  {
    
 Serial.println("dir detected");
 if(!strcmp(menu_strings[menu_current],"..")){
   Serial.println(".. dir detected");
  for(int t=strlen(cdir)-1;t>=1;t--){
    if(cdir[t-1]=='/'){
      cdir[t]='\0';
      break;
    }
  }
    Serial.println("new cdir ");
    Serial.println(cdir);
    
 }else
     sprintf(cdir, "%s%s/",cdir,menu_strings[menu_current]); 
   
  file.close();
  updateFilesList();
  fsmState=0;
  start_menu_items=0;
    menu_current=0;
  return;
  }
  
   
  
 bool exit=false;  
     char lastSym;
   while ((n = file.read(buf2, sizeof(buf2)))) {
   //Serial.write(buf, n);
   for(int ii=0;ii<n;ii++)
   {
    char t=buf2[ii];       
      if(isspace(t) && !isspace( lastSym))totalWords++;
      
      lastSym=t;
    }  
  }
  
  line1="";
  line1Changed=true;
  file.close();
    Serial.print("total words : ");
    Serial.println(totalWords);
 file = sd.open(buffer);
 Serial.println(file);
 exit=false;
 //file.seek(2000);
  while ((n = file.read(buf, sizeof(buf)))) {
   Serial.write(buf, n);
   //for(int ii=0;ii<n;ii++)
   //{
    char t=buf[0];      
    Serial.print(t); 
      if(t==' '){exit=true;break;}
      line1+=t;
   // }
  //  if(exit)break;
  int pos=file.curPosition();
  }
  line1Changed=true;
  Serial.print("first word : ");
  Serial.println(line1);
  line6="";line5="";line4="";line3="";line2="";
  spriteClearRequired=true;
  hasBookmark=getBookmark()>=0;
 if(hasBookmark){
  Serial.println("fsmState 11 ");
  fsmState=11;
 }
  
}

void initMenu()
{
   Serial.print("Initializing SD card...");
    while (!Serial) {
    yield();
  }

  if (!sd.begin(SD_CONFIG)) {
    sdError=true;
    sd.initErrorHalt(&Serial);
  }
  updateFilesList();
}

void updateFilesList()
{
  Serial.println("updateFilesList");
    Serial.println(cdir);
   
  if (!dir.open(cdir)){
    error("dir.open failed");
  }
 
  
  int cntr=0;
   for(int j=0;j<MENU_ITEMS;j++){
    delete [] menu_strings[j];
   }
  MENU_ITEMS=0;
   //if(!dir.isRoot())
   if(strcmp(cdir,"/"))
   {
      MENU_ITEMS++;
       menu_strings[cntr]=new char[strlen("..")+1];
      strcpy(menu_strings[cntr],"..");
      cntr++;
  }
  //char* f_name;
    int max_characters = 80;
    char f_name[max_characters];
    while (file.openNext(&dir, O_RDONLY)) {
     
    file.printName(&Serial);
     Serial.println();
    

      file.getName(f_name, max_characters);
       
       int len=strlen(f_name);

      if(len<70 && len>0)
      {
          if(cntr<100)
          {
          
            menu_strings[cntr]=new char[strlen(f_name)+1];
            strcpy(menu_strings[cntr],f_name);  
            
            MENU_ITEMS++;  
             Serial.print("stored: ");
            Serial.println(f_name);
          
          }
          cntr++;
      }else {
           Serial.print("skipped ");    
        }
        
        //file.printName(&menu_strings[cntr]);
        if (file.isDir()) {
          // Indicate a directory.
          Serial.write('/');
        }
        Serial.println();
        file.close();
    }
    
  if (dir.getError()) {
    Serial.println("openNext failed");
  } else {
    Serial.println("Done!");
  }
  dir.close();
  menu_redraw_required = 1;
}
void uiStep(void) 
{
  String button, button2, pressed_button;  
  button = getButton();
  if (button != prev_key_id)
  {
    delay(50);        // debounce
    button2 = getButton();

    if (button == button2)
    {
      prev_key_id = button;
      pressed_button = button;
      Serial.println(pressed_button);
      //Serial.println(key_read);
    if(pressed_button=="DOWN"){
       
         menu_current++;
         if(menu_current>=(start_menu_items+max_menu_items)){
            start_menu_items=menu_current-max_menu_items+1;
          }
      if ( menu_current >= MENU_ITEMS ){
        menu_current = 0;
        start_menu_items=0;
        }
      menu_redraw_required = 1;}
    if(pressed_button=="UP"){
         if ( menu_current == 0 ){
            menu_current = MENU_ITEMS;
             start_menu_items=menu_current-max_menu_items;
             if(start_menu_items<0)start_menu_items=0;
        }
      menu_current--;  
      
      if(menu_current<(start_menu_items)){
          start_menu_items=menu_current;
          }
      menu_redraw_required = 1;}
    if(pressed_button=="ENTER"){
        Serial.println("enter");
        fsmState=1;
         //digitalWrite(tftCS, HIGH);
       //  digitalWrite(SD_CS_PIN, LOW);  
      setupBook();
      // digitalWrite(tftCS, LOW);
       // digitalWrite(SD_CS_PIN, HIGH);  
      menu_redraw_required=1;
    }
      }
   }       
}

void bookBtnsUpd()
{
  String button, button2, pressed_button;  
    button = getButton();
    if (button != prev_key_id)
          {
              delay(50);        // debounce
              button2 = getButton();
        
              if (button == button2)
              {
                 prev_key_id = button;
                 pressed_button = button;
                     last_pressed_button = button;
                 
                 Serial.println(pressed_button);
                  //Serial.println(key_read);
           menu_redraw_required = 1;
                         if(pressed_button=="LEFT"){
                          
                                    fsmState=2;//menu
                                    pauseBook=true;
                                Serial.println("fsmstate switch 2");
                                 /*file.close();
                                 menu_redraw_required = 1;*/
                       }
                         //if(key_id=="RIGHT")xx++;
                         if(pressed_button=="DOWN"){
                             
                                       speed-=50;
                                  if ( speed < 50 )
                                    speed = 50;
                             } if(pressed_button=="RIGHT"){

                             if(textReadMode==2){
                              //next page
                              nextPageGo=true;
                              
                             }else{
                              if(pauseBook)nextPageGo=true;else
                                     wordsSkip=100;}
                             }
                          if(pressed_button=="UP"){
                             
                                    speed+=50;
                            if ( speed >= 1000 )
                              speed = 1000;
                            }
                          if(pressed_button=="ENTER"){
                          pauseBook=!pauseBook;
                      }
              }
           }  
  
}


int bookMenuSelected=0;
int resumeBookMenuSelected=0;
int currentGoto=0;
bool rewindRequired=false;
int gotoMenuItem=0;

void resumeBookBtnsUpd()
{
  String button, button2, pressed_button;  
    button = getButton();
          if (button != prev_key_id)
          {
              delay(50);        // debounce
              button2 = getButton();
        
              if (button == button2)
              {
                 prev_key_id = button;
                 pressed_button = button;
                 Serial.println(pressed_button);
                  //Serial.println(key_read);
          
                            
                            menu_redraw_required=1;
                         //if(key_id=="RIGHT")xx++;
                         if(pressed_button=="DOWN"){
                             resumeBookMenuSelected++;
                                      resumeBookMenuSelected%=2;
                             } 
                          if(pressed_button=="UP"){
                              resumeBookMenuSelected--;
                                     if( resumeBookMenuSelected<0){
                                      resumeBookMenuSelected=1;
                                     }
                            }
                          if(pressed_button=="ENTER"){
                          if(resumeBookMenuSelected==0){
                            loadBookmark();
                            fsmState=1;
                            //load book
                            
                            menu_redraw_required=1;
                          }else if(resumeBookMenuSelected==1){
                            //Serial.println("fsmState 1. goto to start book");
                            fsmState=1;                           
                                 
                                 menu_redraw_required = 1;
                          }
                      }
              }
           }    
}


int contrast=255;
void bookBtnsUpd2()
{
 String button, button2, pressed_button;  
          button = getButton();
          if (button != prev_key_id)
          {
              delay(50);        // debounce
              button2 = getButton();
        
              if (button == button2)
              {
                 prev_key_id = button;
                 pressed_button = button;
                 Serial.println(pressed_button);
                  //Serial.println(key_read);
          
                        menu_redraw_required=1;
                         //if(key_id=="RIGHT")xx++;
                         if(pressed_button=="DOWN"){
                             bookMenuSelected++;
                                      bookMenuSelected%=8;
                                          
                             } 
                          if(pressed_button=="UP"){
                              bookMenuSelected--;
                                     if( bookMenuSelected<0){
                                      bookMenuSelected=7;
                                     }
                            }
                          if(pressed_button=="ENTER"){
                          if(bookMenuSelected==0){
                            fsmState=1;
                            menu_redraw_required=1;
                          }else if(bookMenuSelected==1){
                            fsmState=0;                           
                                 file.close();
                                 menu_redraw_required = 1;
                          }else if(bookMenuSelected==2){//go to word
                            fsmState=21;                           
                                 currentGoto=wordReaded;
                                 menu_redraw_required = 1;
                          }else if(bookMenuSelected==3){//save bookmark
                            saveBookmark();
                                        fsmState=1;              
                          }else if(bookMenuSelected==4){//load bookmark
                            loadBookmark();
                                        fsmState=1;              
                          }else if(bookMenuSelected==5){//change read mode
                            
                                        textReadMode++;
                                        textReadMode%=3;
                          }else if(bookMenuSelected==6){//change text size
                            
                                        textSize++;
                                        textSize%=3;
                          }else if(bookMenuSelected==7){//contrast
                            if(contrast==64){
                                        contrast=128;}else if(contrast==128){
                                           contrast=255;
                                        }else
                                        contrast=64;
                                        
                                        ledcWrite(0, contrast);
                                       //  u8g2 . setContrast ( contrast) ;
                                         Serial.print("set contrast: ");
                                         Serial.print(contrast);
                          }
                      }
              }
           }    
}

void gotoBtnsUpd()
{   
  String button, button2, pressed_button;  
          button = getButton();
          if (button != prev_key_id)
          {
              delay(50);        // debounce
              button2 = getButton();
        
              if (button == button2)
              {
                 prev_key_id = button;
                 pressed_button = button;
                 Serial.println(pressed_button);
                  //Serial.println(key_read);
          
                        
                         //if(key_id=="RIGHT")xx++;
                         if(pressed_button=="DOWN"){
                                 currentGoto+=100;
                             }   if(pressed_button=="LEFT"){
                                   fsmState=2;                           

                                 menu_redraw_required = 1;
                             } 
                          if(pressed_button=="UP"){
                              currentGoto-=100;
                                     if( currentGoto<0){
                                      currentGoto=0;
                                     }
                            }
                          if(pressed_button=="ENTER"){
                        //  if(gotoMenuItem==0){
                            
                            // fsmState=2;                           

                               //  menu_redraw_required = 1;
                        //  }else if(gotoMenuItem==1){
                             fsmState=1;//rewind here!!!
                             rewindRequired=true;
                             file.seek(0);
                             wordReaded=0;
                             wordsSkip=currentGoto;
                          //}
                      }
              }
           }    
}

int yShift=0;
int xShift=0;

void drawGoto()
{
  //u8g2.clearBuffer();       
  
  gotoBtnsUpd();
  char buffer[80];
  int perc=(int)((wordReaded/(float)totalWords)*100);
  sprintf(buffer, "  %5d/%ld ", wordReaded, totalWords);

  //u8g2.setFont(u8g2_font_6x13_t_cyrillic);
  //u8g2.drawUTF8( 5, 15, BACK_STRING);
  
  //u8g2.drawStr( 5, 35, buffer);
  sprintf(buffer, " goto: %5d ", currentGoto);

  //u8g2.drawStr( 5, 50, buffer);
 // u8g2.sendBuffer();        
}

void drawBook()
{
  tft.setTextSize(0);
   setMyFont();
    //Serial.println("#drawBook");
  //u8g2.clearBuffer();       
  if(pauseBook)
     tft.fillScreen(TFT_BLACK);
     
      tft.setTextColor(TFT_WHITE);
  char buffer[80];
  char buffer2[80];
  int perc=(int)((wordReaded/(float)totalWords)*100);
  sprintf(buffer, "%6d/%ld (%d%c)", wordReaded, totalWords,perc,'%');  
   sprintf(buffer2, "bookmark[%c] s%d",   hasBookmark?'x':' ', speed);   
  first=false;  
  //u8g.setFont(rus8x13); 
  int fw=9;
 
  //u8g2.setFont(u8g2_font_cu12_t_cyrillic);  
  //u8g2.setFont(u8g2_font_6x13_t_cyrillic);  
  if(textSize==0)
  {
    fw=6;  
    //u8g2.setFont(u8g2_font_6x13_t_cyrillic); 
    }
  else if(textSize==1)
  {
    fw=9;  
   // u8g2.setFont(u8g2_font_9x15_t_cyrillic);  
  } else if(textSize==2)
  {
    fw=8;  
    //u8g2.setFont(u8g2_font_8x13_t_cyrillic);  
  }
       int cw=(line1.length()*fw/4);
      
     if(textReadMode==1  )
   {     
    //u8g2.drawUTF8( 64-cw, 42, line1.c_str());
     }
   else
   {
    /*int cw2=(line2.length()*fw/4);
    int cw3=(line3.length()*fw/4);
    int cw4=(line4.length()*fw/4);
    int cw5=(line5.length()*fw/4);
    int cw6=(line6.length()*fw/4);*/
   // u8g2.setDrawColor(1);

     
    int yShift2=13-yShift;
    int height=12;
    int cw6=0;
    int cw5=0;
    int cw4=0;
    int cw3=0;
    int cw2=0;
    int cw=0;
  /*  int cw6=u8g2.getUTF8Width(line6.c_str())/2;
  int  cw5=u8g2.getUTF8Width(line5.c_str())/2;
   int   cw4=u8g2.getUTF8Width(line4.c_str())/2;
   int   cw3=u8g2.getUTF8Width(line3.c_str())/2;
   int   cw2=u8g2.getUTF8Width(line2.c_str())/2;
      cw=u8g2.getUTF8Width(line1.c_str())/2;
    u8g2.drawUTF8(  64-cw6,25+yShift2-height*2, line6.c_str());
    u8g2.drawUTF8(  64-cw5,25+yShift2-height, line5.c_str());
    u8g2.drawUTF8(  64-cw4,25+yShift2, line4.c_str());
    u8g2.drawUTF8(  64-cw3,25+height+yShift2, line3.c_str());
    u8g2.drawUTF8(  64-cw2,25+height*2+yShift2, line2.c_str());
    u8g2.drawUTF8(  64-cw,25+height*3+yShift2, line1.c_str());*/
    
   /* tft.setCursor(64-cw6,25+yShift2-height*2);
  tft.print(line6.c_str());

  tft.setCursor(64-cw5,25+yShift2-height*2);
  tft.print(line5.c_str());
  
    tft.setCursor(64-cw4,25+yShift2-height*2);
  tft.print(line4.c_str());

    tft.setCursor(64-cw3,25+yShift2-height*2);
  tft.print(line3.c_str());

    tft.setCursor(64-cw2,25+height*2+yShift2);
  tft.print(line2.c_str());*/
  tft.setTextDatum(TC_DATUM);
 
stext1.setTextDatum(TC_DATUM);
   stext1.pushSprite(0, 0);
  
    

    
       if(line1.length()>0 && line1Changed){
        if(spriteClearRequired){
          stext1.fillSprite(TFT_BLACK);
          spriteClearRequired=false;
        }
       int padding = stext1.textWidth(line1.c_str(), 1); // get the width of the text in pixels
       padding/=2;
       line1Changed=false;
  //stext1.setTextColor(TFT_GREEN, TFT_BLUE);
  //stext1.setTextPadding(padding);  
     //stext1.setTextPadding(5);
      stext1.setCursor(64-padding,25+yShift2+height*5);
      
  //stext1.println(data);
  stext1.scroll(0,-stext1.fontHeight(1));
    //stext1.drawString(line1.c_str(), 64, 128, 1);
    stext1.print(line1.c_str());
       }
       
    
            /*
    if(line6.length()>0){
      
        int padding = tft.textWidth(line6.c_str(), 1);
        
        cw6=padding/2;
          tft.setCursor(64-cw6,25+yShift2-height*2);
            //tft.setTextColor(color_txt, color_bg);
            //tft.setTextPadding(padding);
  tft.print(line6.c_str());
  
  }

  if(line5.length()>0){
      int padding = tft.textWidth(line5.c_str(), 1);
        
        cw5=padding/2;
  tft.setCursor(64-cw5,25+yShift2-height);
    
  tft.print(line5.c_str());
  }
  
    if(line4.length()>0){
        int padding = tft.textWidth(line4.c_str(), 1);
        
        cw4=padding/2;
   tft.setCursor(64-cw4,25+yShift2);
  
  tft.print(line4.c_str());
    }
  if(line3.length()>0){
        int padding = tft.textWidth(line3.c_str(), 1);
        
        cw3=padding/2;
  tft.setCursor(64-cw3,25+yShift2+height);
    
  tft.print(line3.c_str());}
   if(line2.length()>0){
      int padding = tft.textWidth(line2.c_str(), 1);
        
        cw2=padding/2;
tft.setCursor(64-cw2,25+height*2+yShift2);
   
  tft.print(line2.c_str());}
   if(line1.length()>0){
     int padding = tft.textWidth(line1.c_str(), 1);
        
        cw=padding/2;
    tft.setCursor(64-cw,25+height*3+yShift2);
   
  tft.print(line1.c_str());
   }*/
     }

      //if(textReadMode==1 || pauseBook){
  if(pauseBook)
  {
     
      tft.setTextFont(0);
     tft.fillRect(0, 0, 160, 30, TFT_BLACK);
    /*u8g2.setDrawColor(0);
     
    u8g2.drawBox(0, 0, 128, 25);
     
    u8g2.setFont(u8g2_font_5x7_mr);
     
    u8g2.setDrawColor(1);
    u8g2.drawHLine(0, 25, 128);
     
    u8g2.drawStr( 5, 12, buffer);
    u8g2.drawStr( 5, 20, bookName.c_str());*/
    tft.drawLine(0, 30, 160, 30, TFT_WHITE);
      tft.setTextColor(TFT_GREEN);
      tft.setCursor(3,2);
  tft.print(buffer);
  
    tft.setCursor(3,10);
  tft.print(buffer2);
  
  tft.setTextColor(TFT_ORANGE);
  tft.setCursor(3,18);
  tft.print(bookName.c_str());
  
  }
  //u8g2.sendBuffer();        
}

void drawBookResumeMenu(void) 
{  

  uint8_t i, h;
  uint8_t w, d;
  
     tft.fillScreen(TFT_BLACK);
     setMyFont();
    //u8g2.clearBuffer();         
    
    //u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
  //u8g2.setFont(u8g2_font_unifont_t_symbols);
    int yshift=30;
     tft.setTextColor(TFT_WHITE);  
  //u8g2.drawGlyph(2,yshift+ resumeBookMenuSelected*8+16, 0x25b7);  
  //u8g2.setFont(u8g2_font_5x7_mr); 
  //u8g2.setFont(u8g2_font_6x13_t_cyrillic); 
   tft.setCursor(2,yshift+ resumeBookMenuSelected*8+16);    
  tft.print(">");
  
  //u8g2.setFont(u8g2_font_5x8_t_cyrillic);  
    //h = u8g2.getFontAscent()-u8g2.getFontDescent();
  //w = u8g2.getWidth();
  //u8g2.drawBox(0, i*h+1, w, h);    
  
  //u8g2.drawUTF8(12, yshift+0+13, RESUME_BOOK_YES); 
  //u8g2.drawUTF8(12,yshift+ 8+13, RESUME_BOOK_NO);  


  if(resumeBookMenuSelected==0){
    tft.setTextColor(TFT_GREEN);  
  }
      tft.setCursor(12, yshift+0+13);    
  tft.print(RESUME_BOOK_YES);
  tft.setTextColor(TFT_WHITE);  
  if(resumeBookMenuSelected==1){
    tft.setTextColor(TFT_GREEN);  
  }
  tft.setCursor(12, yshift+26);    
  tft.print(RESUME_BOOK_NO);
  
  //u8g2.drawUTF8(12,13, RESUME_BOOK_TITLE);  
    //u8g2.drawUTF8(12,20, RESUME_BOOK_TITLE_2);  
     tft.setTextColor(TFT_WHITE);  

   tft.setCursor(12, 13);    
  tft.print(RESUME_BOOK_TITLE);
  tft.setCursor(12, 26);    
  tft.print(RESUME_BOOK_TITLE_2);
  //u8g2.sendBuffer();         
}

void drawBookMenu(void) 
{    
       tft.fillScreen(TFT_BLACK);
  uint8_t i, h;
  uint8_t w, d;
  setMyFont();
  tft.setTextColor(TFT_WHITE);  
    //Serial.println("drawBookMenu");
    //u8g2.clearBuffer();         
    //u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
 // u8g2.setFont(u8g2_font_unifont_t_symbols);
       int mul=13;
 // u8g2.drawGlyph(2, bookMenuSelected*mul+16, 0x25b7); 
  tft.setCursor(2,bookMenuSelected*mul+12);    
  tft.print(">");
  
tft.setTextColor(TFT_WHITE);  
  if(bookMenuSelected==0)    tft.setTextColor(TFT_GREEN);  
  
   tft.setCursor(12, 0+13);    
  tft.print(BACK_STRING);

 
tft.setTextColor(TFT_WHITE);  
  if(bookMenuSelected==1)    tft.setTextColor(TFT_GREEN);  
  
   tft.setCursor(12, mul+13);    
  tft.print(IN_MENU_STRING);

   
tft.setTextColor(TFT_WHITE);  
  if(bookMenuSelected==2)    tft.setTextColor(TFT_GREEN);  
  
   tft.setCursor(12, mul*2+13);    
  tft.print(GOTO_WORD_STRING);
 
tft.setTextColor(TFT_WHITE);  
  if(bookMenuSelected==3)    tft.setTextColor(TFT_GREEN);  

  
    tft.setCursor(12, mul*3+13);    
  tft.print(SAVE_BOOKMARK_STRING);

   
tft.setTextColor(TFT_WHITE);  
  if(bookMenuSelected==4)    tft.setTextColor(TFT_GREEN); 
   
  tft.setCursor(12, mul*4+13);    
  tft.print(LOAD_BOOKMARK_STRING);
  
 
    char buf[100];
    sprintf(buf,"%s: %s",READ_MODE_STRING,textReadMode==0?SCROLL_STRING:((textReadMode==1)?ONE_WORD_STRING:MANUAL_SCROLL_STRING));
     
   
tft.setTextColor(TFT_WHITE);  
  if(bookMenuSelected==5)    tft.setTextColor(TFT_GREEN);  
   tft.setCursor(12, mul*5+13);    
  tft.print(buf);
  sprintf(buf,"%s: %s",FONT_SIZE_STRING, textSize==0?SMALL_FONT_SIZE_STRING:((textSize==1)?BIG_FONT_SIZE_STRING:WIDE_FONT_SIZE_STRING));
 
tft.setTextColor(TFT_WHITE);  
  if(bookMenuSelected==6)    tft.setTextColor(TFT_GREEN);  
   tft.setCursor(12, mul*6+13);    
  tft.print(buf);
    //sprintf(buf,"%s: %s",CONTRAST_STRING, contrast==0?CONTRAST_LOW_STRING:CONTRAST_HIGH_STRING);
   sprintf(buf,"%s: %d",CONTRAST_STRING, contrast);
   
tft.setTextColor(TFT_WHITE);  
  if(bookMenuSelected==7)    tft.setTextColor(TFT_GREEN);  
    tft.setCursor(12, mul*7+13);    
  tft.print(buf);
 
}

void drawMenu(void) 
{
  tft.setTextWrap(false);
   tft.setTextFont(0);
  uint8_t i, h;
  uint8_t w, d;
  //u8g2.setDisplayRotation(U8G2_R2);
  //u8g2 . setContrast ( contrast) ;
    //u8g2.clearBuffer();     

       tft.fillScreen(TFT_BLACK);
    //u8g2.setFont(u8g2_font_ncenB08_tr);
    
//u8g2.setFont(u8g2_font_5x7_mr);
//u8g2.setFont(u8g2_font_5x7_t_cyrillic);
  //delay(1000);  
 
  /*u8g.setFont(u8g_font_6x13);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();*/
  if(sdError){
      //u8g2.drawStr(2, 25, SD_CARD_NOT_FOUND_STRING);  
  }else{
  //h = u8g2.getFontAscent()-u8g2.getFontDescent();
  h=12;
  //w = u8g2.getWidth();
  for( i = 0; i < min(MENU_ITEMS,max_menu_items); i++ ) {
   // d = (w-u8g2.getStrWidth(menu_strings[i+start_menu_items]))/2;
   d=15;
  //  u8g.setDefaultForegroundColor();
    if ( (i+start_menu_items)== menu_current ) {
      //u8g2.drawBox(0, i*h+1, w, h);
     // u8g.setDefaultBackgroundColor();
        //u8g2.setFontMode(1);
     //u8g2.setDrawColor(2);
  tft.setTextColor(TFT_GREEN);   
    }else{
      tft.setTextColor(TFT_WHITE);
        // u8g2.setFontMode(0);
      //u8g2.setDrawColor(1);
    }
      //u8g2.drawUTF8(d, i*h+h, menu_strings[i+start_menu_items]);     
   tft.setCursor(d, i*h);
  
  //tft.setTextWrap(true);
  tft.print(menu_strings[i+start_menu_items]);
  
  }
  }
   //u8g2.sendBuffer();          
}
void loop() {

  if(fsmState==0)
  {
    uiStep();                                     // check for key press    
    if (  menu_redraw_required != 0 ) {    
      drawMenu();   
      menu_redraw_required = 0;
    }  
  }else
   if(fsmState==2)
    {
    bookBtnsUpd2();
    if(fsmState==2 && menu_redraw_required!=0){
      drawBookMenu();
      menu_redraw_required=0;
      }
    } else if(fsmState==21)
    {
      drawGoto();
    }else if(fsmState==11)//yes/no resume book
   {     
    resumeBookBtnsUpd();
    if(fsmState==11 && menu_redraw_required!=0)
      {
        drawBookResumeMenu();
          menu_redraw_required=0;
      } 
    
    
    }else if(fsmState==1)
    {       
      //Serial.println("fsmState#1");
  bookBtnsUpd();
      if(fsmState==1 && menu_redraw_required!=0)
      {
        drawBook(); 
        menu_redraw_required=0;
      } 
    
      int speedSum=speed;
      speedSum+= line1.length()*10;
      if (line1.indexOf('.') !=-1)
        speedSum+=200;
        if(textReadMode==2){
         // speedSum=(speed-50)/20;// horiz scroll
         //   delay(speedSum);
        }else
      if(textReadMode==0)
    {
      speedSum=speed;
      if( (millis()-time1 )<speedSum)
      {
        return;
      }
    time1=millis();
  
  }else
  
   if( (millis()-time1 )<speedSum){
    return;
   }
    time1=millis();
  //textReadMode=0 - scroll
  if(!pauseBook || (nextPageGo && textReadMode==0)){
    
      menu_redraw_required=1;
      int fw=6;
     if( textSize==1)fw=9;
        int cw=(line6.length()/2+1)*fw;
   if(textReadMode==1 ||/* yShift>11 */ textReadMode==0 || (textReadMode==2 && nextPageGo) /*|| xShift>=(cw)*/){
    yShift=0;
    int repeats=1;
    if(nextPageGo)repeats=5;
     nextPageGo=false;
     
    for(int hh=0;hh<repeats;hh++)
  {   
    xShift-=cw;     
    int n;
    char buf[1];
    line6=line5;
    line5=line4;
    line4=line3;
    line3=line2;
    line2=line1;
    int skipped=0;
    while(1)
    {
                line1="";
                line1Changed=true;
                int cntr=0;
                while ((n = file.read(buf, sizeof(buf)))) 
        {
          cntr++;                 
                    if(cntr>100)break;
                 
          char t=buf[0];       
                    if(t==' ' || t=='\t' || t=='\n' || t=='\r'  ){                      
            break;
                     }
                     
                    if(t=='.' || t==',' || (t=='-' && line1.length()>8)){line1+=t; break;}
                    line1+=t;              
                }
                if(n==-1){
                  pauseBook=true;
                  return;
                }
                bool whiteSpacesOnly = std::all_of(line1.begin(),line1.end(),isspace);
                if(!whiteSpacesOnly)
        {
          skipped++;
          if(skipped>=wordsSkip)
          {
            wordReaded+=wordsSkip;
            if(wordsSkip>1){
              line6="";line5="";line4="";line3="";line2="";
              spriteClearRequired=true;
            }
            wordsSkip=1;                   
            break;
          }
        }             
    }
   //Serial.println("line1: ");
   //Serial.println(line1); 
   //Serial.println(line1.length());
  } 
  }}}
    
}


void testdrawtext(char *text, uint16_t color) {
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}


bool dir1;
int accum=0;
int tt=0;

void printMenu(){
  
}

void tftPrintTest() 
{
  tft.setTextWrap(false);
  tft.fillScreen(TFT_BLACK);
  int i=0; 
  char data[80];
  int cntr=2;
 
  stext1.setTextColor(TFT_WHITE);
  stext1.setTextSize(1);
  
   stext1.setCursor(0, 0);
  stext1.println("hi  world 1!"); 
  stext1.setCursor(0, 16);
  stext1.println("hi  world 2!");
    
  while(true){
 
    i++;
    //tft.fillScreen(TFT_BLACK);
 // tft.setCursor(0, 30-i%100);
 
  
  //tft.fillScreen(TFT_BLACK);
  //graph1.pushSprite(0, 64-i%100);
  //stext1.pushSprite(0, 64-i%100);
  stext1.pushSprite(0, 0);
  accum++;
    //if(accum>16)
    {
      tt++;
      //dir=!dir;
      accum=0; 
      //stext1.setCursor(0, 128-16); 
      byte font = 8;
      stext1.setTextFont(1); 
      if(tt%2==0)
        sprintf(data,"АБВГД %d",cntr++);
        else
           sprintf(data,"hello %d",cntr++);
       int padding = stext1.textWidth(data, 1); // get the width of the text in pixels
  //stext1.setTextColor(TFT_GREEN, TFT_BLUE);
  //stext1.setTextPadding(padding);  
     stext1.setTextPadding(5);
  //stext1.println(data);
  stext1.scroll(0,-stext1.fontHeight(1)*2);
    stext1.drawString(data, 64, 128, 1);
    }
     //stext1.scroll(0,dir?1:-1);
     
  delay(200);
  }
  
}
