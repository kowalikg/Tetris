#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glut.h>
#include <gl\glaux.h>
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <cmath>
/*
         Proporcje ekranu zosta³y ustalone na 10x10, a komórka planszy ma wtedy szerokoœæ 0.4.
         Mamy te¿ pewnoœæ, ¿e nie zostanie wylosowany kolor bia³y, gdy¿ wyrzuca siê go z losowania koloru (case 0).
         W pliku ".h" deklarujemy tylko te funkcje, które zastosujemy w main.cpp.
*/
int x,y;//pomocnicze wspó³rzêdne
int score=0;//wynik
int alpha=0;//bêdziemy rozpatrywaæ przypadki obrotu (uwaga: klocki obracaj¹ siê wzglêdem "punktu 0")
int level=1;//licznik poziomu - z ka¿dym poziomem wzrasta szybkoœæ spadania

int brickPosX=7;//wspó³rzêdne gdzie ma siê zaczynaæ spadanie klocka
int brickPosY=17;

int currentBrickShape;//zmienna do losowania kszta³tu
int currentBrickColor;//zmienna do losowania koloru

int nextBrickShape;//losowanie nastêpnego kszta³tu i koloru
int nextBrickColor;

const int brickShapes[7][4][2] = // tablica w której zapisane s¹ kszta³ty klocków
{
  {
    {  0,  0 }, // ....
    {  1,  0 }, // .XX.
    {  0,  1 }, // .XX.
    {  1,  1 }  // ....
  },
  {
    {  0, -1 }, // .X..
    {  0,  0 }, // .X..
    {  0,  1 }, // .X..
    {  0,  2 }  // .X..
  },
  {
    {  0,  0 }, // ....
    {  1,  0 }, // .XX.
    {  0,  1 }, // .X..
    {  0,  2 }  // .X..
  },
  {
    {  0,  0 }, // ....
    { -1,  0 }, // XX..
    {  0,  1 }, // .X..
    {  0,  2 }  // .X..
  },
  {
    {  0, -1 }, // .X..
    {  0,  0 }, // .XX.
    {  1,  0 }, // ..X.
    {  1,  1 }  // ....
  },
  {
    {  1, -1 }, // ..X.
    {  1,  0 }, // .XX.
    {  0,  0 }, // .X..
    {  0,  1 }  // ....
  },
  {
    { -1,  0 }, // ....
    {  0,  0 }, // XXX. 
    {  1,  0 }, // .X..
    {  0,  1 }  // ....
  }
};

const int keys_time=50;//czas w którym nie mo¿emy wykonaæ kolejnego naciœniêcia

long last_fall=0;
long startTime;

const float FIELD_WIDTH = 0.4f;//szerokoœæ komórki
float fall_time=500;//szybkoœæ pocz¹tkowego spadania klocka

bool game_over=false;//warunek koñcz¹cy grê

int fields[14][20];//plansza

int check_line(int fields[][20])//funkcja sprawdza czy linia jest zape³niona
{   
    int block=0;//zmienna licz¹ca ile jest zape³nionych komórek w wierszu
    int y=0; //wspó³rzêdna
    int count=0;//licznik zwracaj¹cy liczbê pe³nych lini
    
    while(y<20)
    {
               for (int x=0;x<14;x++)
               {   
                   if (fields[x][y]!=0)
                   block++;//licznik sprawdzi czy WSZYSTKIE komórki s¹ zape³nione
               }
               if (block==14)//jeœli s¹
               {
                           for(int i=0;i<14;i++)
                           {
                                   for (int j=(y+1);j<20;j++)
                                   fields[i][j-1]=fields[i][j];//zrzucamy w dó³ linie
                                      
                           } 
               
               count++;           
                           
               }
               else      
               y++;
               block=0;
  }
  
   return count;
}

void change_xy(int alpha, int i)//zmieniamy wspó³rzêdne w zale¿noœci od obrotu
{
     switch (alpha)
              {
     
              case 1:
          
              x = brickPosX + brickShapes[currentBrickShape][i][1];
              y = brickPosY - brickShapes[currentBrickShape][i][0];
              break;
    
              case 2:
          
              x = brickPosX - brickShapes[currentBrickShape][i][0];
              y = brickPosY - brickShapes[currentBrickShape][i][1];
              break;
     
              case 3:  
          
              x = brickPosX - brickShapes[currentBrickShape][i][1];
              y = brickPosY + brickShapes[currentBrickShape][i][0];
              break;
     
              default: 
              x = brickPosX + brickShapes[currentBrickShape][i][0];
              y = brickPosY + brickShapes[currentBrickShape][i][1];
              break;
     
              }
}

bool brick_correct(int brickPosX, int brickPosY, const int brickShapes[][2], int alpha, int pos, bool is_left,int fields[][20])
{//funkcja sprawdza, czy mo¿emy klocek przesun¹æ
     bool result=true;
     for (int i=0; i<4; i++)
     {  
        change_xy(alpha,i);
        
        if (is_left)//sprawdzamy, która krawêdŸ
        {
         if ((x<pos)||(fields[x-1][y]>0))
         result=false;
        }
     
        else
        {
         if ((x>pos)||(fields[x+1][y]>0))
         result=false;
        }
    }
    return result;
}

bool brick_can_fall(int brickPosX, int brickPosY, const int brickShapes[][2], int alpha, int fields[][20])//funkcja sprawdza, czy klocek mo¿e spaœæ jeszcze w dó³
{
  bool result=true;// z góry zak³adamy ¿e tak
  for (int i=0; i<4; i++)
  {  
     change_xy(alpha,i);
     if ((y<1)||(fields[x][y-1]>0))//je¿eli nie ma jak
     result=false;//to siê zatrzymuje
     
     
  }
  return result;
}

void clear()//tê funkcjê wywo³ujemy tylko raz, aby unikn¹æ podstawienia na ni¿ej wymienione zmienne losowych wartoœci
{
     
     
      for (int x=0; x<10; x++) 
      {
          for (int y=0; y<20; y++) 
          {
              fields[x][y] = false;
          }
      }
      
  startTime = GetTickCount();
  currentBrickShape = rand() % 7;//losujemy kszta³ty
  nextBrickShape = rand() % 7;
  currentBrickColor = (rand() % 8) + 1;//i kolory
  nextBrickColor = (rand() % 8) + 1;
  
}

void rotate()//obracamy klockiem
{
     if(brick_correct(brickPosX,brickPosY,brickShapes[currentBrickShape],alpha+1,0,true,fields))//oczywiœcie jeœli mo¿emy to zrobiæ
        {            
          if(brick_correct(brickPosX,brickPosY,brickShapes[currentBrickShape],alpha+1,13,false,fields))
          {
            if (alpha==3)
            {
                           alpha=-1; 
            }
            alpha++;
          }
        }
}   
 
void get_down()//zni¿amy na sam dó³
{   
         while(brick_can_fall(brickPosX, brickPosY, brickShapes[currentBrickShape],alpha,fields))
         brickPosY--;
             
}

void go_left()//ruszamy w lewo
{
     if (brick_correct(brickPosX,brickPosY,brickShapes[currentBrickShape],alpha,1,true,fields))  //jeœli mo¿emy, to przesuwamy w lewo          
          brickPosX--;
}

void go_right()//i w prawo
{
     if (brick_correct(brickPosX,brickPosY,brickShapes[currentBrickShape],alpha,12,false,fields))
     brickPosX++;
}

void draw_colors(int fields)//rysujemy kolory
{
     switch (fields)
         {
          case 0:
          glColor3f(1, 1, 1);    
          break;
          case 1:
          glColor3f(1, 0, 0);
          break;
          case 2:     
          glColor3f(1, 0.0784314, 0.576471);
          break;
          case 3:     
          glColor3f(1, 0.647059, 0);
          break;
          case 4:     
          glColor3f(1, 1, 0);
          break;
          case 5:     
          glColor3f(0, 1, 0);
          break;
          case 6:     
          glColor3f(0, 1, 1);
          break;
          case 7:     
          glColor3f(0, 0.74902, 1);
          break;
          case 8:     
          glColor3f(0.627451, 0.12549, 0.941176);
          break;       
        }
}

void draw_next()//rysujemy nastêpnego klocka
{   
    draw_colors(nextBrickColor);
    const float next_x=(18-(20*FIELD_WIDTH))/2;
    const float next_y=(12-(20*FIELD_WIDTH))/2;
    for (int i=0;i<4;i++)
    {
     const int x = 7 + brickShapes[nextBrickShape][i][0];
     const int y = 7 + brickShapes[nextBrickShape][i][1];
    
     glBegin(GL_QUADS);
     glVertex3f(next_x+(x)*FIELD_WIDTH, next_y+(y)*FIELD_WIDTH, 0);
     glVertex3f(next_x+(x+1)*FIELD_WIDTH, next_y+(y)*FIELD_WIDTH, 0);
     glVertex3f(next_x+(x+1)*FIELD_WIDTH, next_y+(y+1)*FIELD_WIDTH, 0);
     glVertex3f(next_x+(x)*FIELD_WIDTH, next_y+(y+1)*FIELD_WIDTH, 0);
     glEnd();
    }
}

int DrawGlScene()//g³ówna funkcja
{
    

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  
  long currentTime = GetTickCount()-startTime;//obliczamy czas
  
  float orgin_x=(7-(14*FIELD_WIDTH))/2;
  float orgin_y=(10-(20*FIELD_WIDTH))/2;

  
  if (currentTime - last_fall>fall_time)//sprawdzamy czy nie za wczeœnie za ruch
  {
      last_fall = currentTime;
      
      if (brick_can_fall(brickPosX, brickPosY, brickShapes[currentBrickShape],alpha,fields))//jeœli mo¿e spadaæ, niech klocek dalej spada
      {
         brickPosY--;
         
      }
      else //je¿eli ju¿ nie mo¿e, zapisujemy jego kszta³t do tablicy
      {   
          for (int i=0;i<4;i++)
           {
              change_xy(alpha,i);//uwzglêdniaj¹c obrót
                   
              switch (currentBrickColor)//pod postaci¹ int, dziêki którym mo¿emy narysowaæ kolory
              {
                 case 1:
                 fields[x][y] = 1;
                 break;
                 
                 case 2:     
                 fields[x][y] = 2;
                 break;
                 
                 case 3:     
                 fields[x][y] = 3;
                 break;
                 
                 case 4:     
                 fields[x][y] = 4;
                 break;
                 
                 case 5:     
                 fields[x][y] = 5;
                 break;
                 
                 case 6:     
                 fields[x][y] = 6;
                 break;
                 
                 case 7:     
                 fields[x][y] = 7;
                 break;
                 
                 case 8:     
                 fields[x][y] = 8;
                 break;
              }
      
          }   
          
          //resetujemy zmienne
          currentBrickShape = nextBrickShape;//podstawiamy
          nextBrickShape = rand() % 7;//i losujemy nastêpn¹
          currentBrickColor = nextBrickColor;
          nextBrickColor = (rand() % 8) + 1;
          
          brickPosY=17;
          brickPosX=7;
          alpha=0;
          
          
            int points = check_line(fields);//sprawdzamy czy uda³o siê zape³niæ któr¹œ z liñ         
            score+=10*points;//jeœli tak to dodajemy punkty
            level=(score/100)+1;//obliczamy na którym jesteœmy poziomie (co 100 punktów level++)
            fall_time=500*pow(0.7,level);//zwiêkszamy szybkoœæ
                 
                 
                 
          if(!brick_can_fall(brickPosX, brickPosY, brickShapes[currentBrickShape],alpha,fields))//je¿eli nie mo¿na zrzuciæ nastêpnego klocka, koniec gry
          game_over=true;
     }
  }
  
  
  for (int i=0; i<14; i++) //rysujemy planszê uwzglêdniaj¹c szerokoœæ ekranu
   {
    for (int j=0; j<20; j++) 
    {   
        draw_colors(fields[i][j]);
        
        glBegin(GL_QUADS);
        glVertex3f(orgin_x+(i)*FIELD_WIDTH, orgin_y+(j+1)*FIELD_WIDTH, 0);
        glVertex3f(orgin_x+(i+1)*FIELD_WIDTH, orgin_y+(j+1)*FIELD_WIDTH, 0);
        glVertex3f(orgin_x+(i+1)*FIELD_WIDTH, orgin_y+(j)*FIELD_WIDTH, 0);
        glVertex3f(orgin_x+(i)*FIELD_WIDTH, orgin_y+(j)*FIELD_WIDTH, 0);
        glEnd();
    }
    
  }
  
  
  for (int i=0; i<4; i++)//ustalamy wspó³rzêdne klocka
  {  
    change_xy(alpha,i);  
    draw_colors(currentBrickColor);
    
    glBegin(GL_QUADS);
    glVertex3f(orgin_x+(x)*FIELD_WIDTH, orgin_y+(y)*FIELD_WIDTH, 0);
    glVertex3f(orgin_x+(x+1)*FIELD_WIDTH, orgin_y+(y)*FIELD_WIDTH, 0);
    glVertex3f(orgin_x+(x+1)*FIELD_WIDTH, orgin_y+(y+1)*FIELD_WIDTH, 0);
    glVertex3f(orgin_x+(x)*FIELD_WIDTH, orgin_y+(y+1)*FIELD_WIDTH, 0);
    glEnd();
    
    
 }
 
 if (game_over)//sprawdzamy czy mo¿na dalej graæ
 {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  return 0;
 }
 else
 {
  glBegin(GL_QUADS);
  glColor3f(1, 1, 1);
  glVertex3f(7, 9,0);
  glVertex3f(7, 1,0);
  glVertex3f(9, 1,0);
  glVertex3f(9, 9,0);
  glEnd();  
  draw_next();
  return 1;
 }
}

