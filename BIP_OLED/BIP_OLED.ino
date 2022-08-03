  /*
 Book In Pocket OLED
*/
#include "strings_rus.h"
 
#include "SdFat.h"

String prev_key_id;
// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 0

#include <Arduino.h>
#include <U8g2lib.h>

#include <SPI.h>


#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_DC     16
#define OLED_CS     5
#define OLED_RESET  17

U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ OLED_CS, /* dc=*/ OLED_DC, /* reset=*/ OLED_RESET);

#define SDCARD_SS_PIN 4
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  

#define SPI_CLOCK SD_SCK_MHZ(25)
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SPI_CLOCK)

int key=34;

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
ExFile fileB;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile dir;
FsFile file;
FsFile fileB;
#else  // SD_FAT_TYPE
#error invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE
#include <SPI.h>

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
uint8_t menu_redraw_required = 0;

int fsmState=0;//0 - main menu ; 1- book read mode, 2-book read menu
bool pauseBook=false;


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

int wordsSkip=1;
String last_pressed_button;
bool nextPageGo=false;

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
                            if(contrast==0){
                                        contrast=255;}else
                                        contrast=0;
                                        
                                         u8g2 . setContrast ( contrast) ;
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
	u8g2.clearBuffer();       
  
	gotoBtnsUpd();
	char buffer[80];
	int perc=(int)((wordReaded/(float)totalWords)*100);
	sprintf(buffer, "  %5d/%ld ", wordReaded, totalWords);

	u8g2.setFont(u8g2_font_6x13_t_cyrillic);
	u8g2.drawUTF8( 5, 15, BACK_STRING);
  
	u8g2.drawStr( 5, 35, buffer);
	sprintf(buffer, " goto: %5d ", currentGoto);

	u8g2.drawStr( 5, 50, buffer);
	u8g2.sendBuffer();        
}

bool hasBookmark=false;

void drawBook()
{
	u8g2.clearBuffer();       
  
	char buffer[80];
	int perc=(int)((wordReaded/(float)totalWords)*100);
	sprintf(buffer, "%6d/%ld (%d%c) %c%c%d", wordReaded, totalWords,perc,'%', pauseBook?'p':' ', hasBookmark?'b':' ', speed);    
	first=false;  
	//u8g.setFont(rus8x13); 
	int fw=9;
 
	u8g2.setFont(u8g2_font_cu12_t_cyrillic);  
	u8g2.setFont(u8g2_font_6x13_t_cyrillic);  
	if(textSize==0)
	{
		fw=6;  
		u8g2.setFont(u8g2_font_6x13_t_cyrillic); 
    }
	else if(textSize==1)
	{
		fw=9;  
		u8g2.setFont(u8g2_font_9x15_t_cyrillic);  
	} else if(textSize==2)
	{
		fw=8;  
		u8g2.setFont(u8g2_font_8x13_t_cyrillic);  
	}
       int cw=(line1.length()*fw/4);
      
     if(textReadMode==1  )
	 {		 
		u8g2.drawUTF8( 64-cw, 42, line1.c_str());
     }
	 else
	 {
		/*int cw2=(line2.length()*fw/4);
		int cw3=(line3.length()*fw/4);
		int cw4=(line4.length()*fw/4);
		int cw5=(line5.length()*fw/4);
		int cw6=(line6.length()*fw/4);*/
		u8g2.setDrawColor(1);

     
		int yShift2=13-yShift;
		int height=12;
    int cw6=u8g2.getUTF8Width(line6.c_str())/2;
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
		u8g2.drawUTF8(  64-cw,25+height*3+yShift2, line1.c_str());
     }

      //if(textReadMode==1 || pauseBook){
	if(pauseBook)
	{
		u8g2.setDrawColor(0);
     
		u8g2.drawBox(0, 0, 128, 25);
     
		u8g2.setFont(u8g2_font_5x7_mr);
     
		u8g2.setDrawColor(1);
		u8g2.drawHLine(0, 25, 128);
     
		u8g2.drawStr( 5, 12, buffer);
		u8g2.drawStr( 5, 20, bookName.c_str());
	}
	u8g2.sendBuffer();        
}

void drawBookResumeMenu(void) 
{  

	uint8_t i, h;
	u8g2_uint_t w, d;
	
    
    u8g2.clearBuffer();         
    //u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
	u8g2.setFont(u8g2_font_unifont_t_symbols);
    int yshift=26;
	u8g2.drawGlyph(2,yshift+ resumeBookMenuSelected*8+16, 0x25b7);  
	//u8g2.setFont(u8g2_font_5x7_mr); 
	u8g2.setFont(u8g2_font_6x13_t_cyrillic); 
	
	u8g2.setFont(u8g2_font_5x8_t_cyrillic);  
    h = u8g2.getFontAscent()-u8g2.getFontDescent();
	w = u8g2.getWidth();
	//u8g2.drawBox(0, i*h+1, w, h);    
	
	u8g2.drawUTF8(12, yshift+0+13, RESUME_BOOK_YES); 
	u8g2.drawUTF8(12,yshift+ 8+13, RESUME_BOOK_NO);  
      
	u8g2.drawUTF8(12,13, RESUME_BOOK_TITLE);  
    u8g2.drawUTF8(12,20, RESUME_BOOK_TITLE_2);  
    
	u8g2.sendBuffer();         
}

void drawBookMenu(void) 
{  
	uint8_t i, h;
	u8g2_uint_t w, d;
	
    //Serial.println("drawBookMenu");
    u8g2.clearBuffer();         
    //u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
	u8g2.setFont(u8g2_font_unifont_t_symbols);
       int mul=7;
	u8g2.drawGlyph(2, bookMenuSelected*mul+16, 0x25b7); 
	//u8g2.setFont(u8g2_font_5x7_mr); 
	u8g2.setFont(u8g2_font_6x13_t_cyrillic); 
	//delay(1000);  
	u8g2.setFont(u8g2_font_5x8_t_cyrillic);  

  
	u8g2.drawBox(0, i*h+1, w, h);    
	u8g2.drawUTF8(12, 0+13, BACK_STRING); 
	u8g2.drawUTF8(12, mul+13, IN_MENU_STRING);  
	u8g2.drawUTF8(12, mul*2+13, GOTO_WORD_STRING);  
	u8g2.drawUTF8(12, mul*3+13, SAVE_BOOKMARK_STRING); 
	u8g2.drawUTF8(12, mul*4+13, LOAD_BOOKMARK_STRING);  
    char buf[100];
    sprintf(buf,"%s: %s",READ_MODE_STRING,textReadMode==0?SCROLL_STRING:((textReadMode==1)?ONE_WORD_STRING:MANUAL_SCROLL_STRING));
     
    u8g2.drawUTF8(12, mul*5+13, buf);  
	sprintf(buf,"%s: %s",FONT_SIZE_STRING, textSize==0?SMALL_FONT_SIZE_STRING:((textSize==1)?BIG_FONT_SIZE_STRING:WIDE_FONT_SIZE_STRING));
    u8g2.drawUTF8(12, mul*6+13, buf);  
    sprintf(buf,"%s: %s",CONTRAST_STRING, contrast==0?CONTRAST_LOW_STRING:CONTRAST_HIGH_STRING);
    u8g2.drawUTF8(12, mul*7+13, buf);  
    
	u8g2.sendBuffer();         
}

bool sdError=false;

void drawMenu(void) 
{
  uint8_t i, h;
  u8g2_uint_t w, d;
  u8g2.setDisplayRotation(U8G2_R2);
  u8g2 . setContrast ( contrast) ;
    u8g2.clearBuffer();          
    u8g2.setFont(u8g2_font_ncenB08_tr);
    
u8g2.setFont(u8g2_font_5x7_mr);
//u8g2.setFont(u8g2_font_5x7_t_cyrillic);
  //delay(1000);  
 
  /*u8g.setFont(u8g_font_6x13);
  u8g.setFontRefHeightText();
  u8g.setFontPosTop();*/
  if(sdError){
      u8g2.drawStr(2, 25, SD_CARD_NOT_FOUND_STRING);  
  }else{
  h = u8g2.getFontAscent()-u8g2.getFontDescent();
  w = u8g2.getWidth();
  for( i = 0; i < min(MENU_ITEMS,max_menu_items); i++ ) {
    d = (w-u8g2.getStrWidth(menu_strings[i+start_menu_items]))/2;
  //  u8g.setDefaultForegroundColor();
    if ( (i+start_menu_items)== menu_current ) {
      u8g2.drawBox(0, i*h+1, w, h);
     // u8g.setDefaultBackgroundColor();
        u8g2.setFontMode(1);
     u8g2.setDrawColor(2);
     
    }else{
         u8g2.setFontMode(0);
      u8g2.setDrawColor(1);
    }
      u8g2.drawUTF8(d, i*h+h, menu_strings[i+start_menu_items]);     
  
  }
  }
   u8g2.sendBuffer();          
}

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
		fileB = sd.open("arduino_bookmarks.txt", FILE_WRITE); 
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
    fileB = sd.open("arduino_bookmarks.txt", FILE_READ);
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
      fileB = sd.open("arduino_bookmarks.txt", FILE_READ);
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
//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(&Serial, F(s))
//------------------------------------------------------------------------------

void setupBook()
{
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
  file.close();
 file = sd.open(buffer);
 exit=false;
 //file.seek(2000);
  while ((n = file.read(buf, sizeof(buf)))) {
   Serial.write(buf, n);
   //for(int ii=0;ii<n;ii++)
   //{
    char t=buf[0];       
      if(t==' '){exit=true;break;}
      line1+=t;
   // }
  //  if(exit)break;
  int pos=file.curPosition();
  }
  
	Serial.print("first word : ");
	Serial.println(line1);
	line6="";line5="";line4="";line3="";line2="";
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

#include <WiFi.h>

void setup() 
{
	Serial.begin (115200);
	setCpuFrequencyMhz(80); //Set CPU clock to 80MHz fo example
	Serial.println();
	Serial.print("CPU Frequency is: ");
	Serial.print(getCpuFrequencyMhz()); //Get CPU clock
	Serial.print(" Mhz");
	Serial.println();
	WiFi.mode(WIFI_OFF);
	btStop();
  
	u8g2.setBusClock(8000000);//"u8g2.setBusClock(1000000);" and "u8g2.setBusClock(8000000);".
   
    u8g2.begin();
    u8g2.enableUTF8Print();
    initMenu();
    pinMode(key,INPUT);
    menu_redraw_required = 1;     // force initial redraw
}

unsigned long time1;

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
			setupBook();
			menu_redraw_required=1;
		}
      }
   }       
}

void loop() 
{  
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
  }}}}
