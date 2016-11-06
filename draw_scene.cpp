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
         Proporcje ekranu zosta�y ustalone na 10x10, a kom�rka planszy ma wtedy szeroko�� 0.4.
         Mamy te� pewno��, �e nie zostanie wylosowany kolor bia�y, gdy� wyrzuca si� go z losowania koloru (case 0).
         W pliku ".h" deklarujemy tylko te funkcje, kt�re zastosujemy w main.cpp.
*/
int x,y;//pomocnicze wsp�rz�dne
int score=0;//wynik
int alpha=0;//b�dziemy rozpatrywa� przypadki obrotu (uwaga: klocki obracaj� si� wzgl�dem "punktu 0")
int level=1;//licznik poziomu - z ka�dym poziomem wzrasta szybko�� spadania

int brickPosX=7;//wsp�rz�dne gdzie ma si� zaczyna� spadanie klocka
int brickPosY=17;

int currentBrickShape;//zmienna do losowania kszta�tu
int currentBrickColor;//zmienna do losowania koloru

int nextBrickShape;//losowanie nast�pnego kszta�tu i koloru
int nextBrickColor;

const int brickShapes[7][4][2] = // tablica w kt�rej zapisane s� kszta�ty klock�w
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

const int keys_time=50;//czas w kt�rym nie mo�emy wykona� kolejnego naci�ni�cia

long last_fall=0;
long startTime;

const float FIELD_WIDTH = 0.4f;//szeroko�� kom�rki
float fall_time=500;//szybko�� pocz�tkowego spadania klocka

bool game_over=false;//warunek ko�cz�cy gr�

int fields[14][20];//plansza

int check_line(int fields[][20])//funkcja sprawdza czy linia jest zape�niona
{   
    int block=0;//zmienna licz�ca ile jest zape�nionych kom�rek w wierszu
    int y=0; //wsp�rz�dna
    int count=0;//licznik zwracaj�cy liczb� pe�nych lini
    
    while(y<20)
    {
               for (int x=0;x<14;x++)
               {   
                   if (fields[x][y]!=0)
                   block++;//licznik sprawdzi czy WSZYSTKIE kom�rki s� zape�nione
               }
               if (block==14)//je�li s�
               {
                           for(int i=0;i<14;i++)
                           {
                                   for (int j=(y+1);j<20;j++)
                                   fields[i][j-1]=fields[i][j];//zrzucamy w d� linie
                                      
                           } 
               
               count++;           
                           
               }
               else      
               y++;
               block=0;
  }
  
   return count;
}

void change_xy(int alpha, int i)//zmieniamy wsp�rz�dne w zale�no�ci od obrotu
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
{//funkcja sprawdza, czy mo�emy klocek przesun��
     bool result=true;
     for (int i=0; i<4; i++)
     {  
        change_xy(alpha,i);
        
        if (is_left)//sprawdzamy, kt�ra kraw�d�
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

bool brick_can_fall(int brickPosX, int brickPosY, const int brickShapes[][2], int alpha, int fields[][20])//funkcja sprawdza, czy klocek mo�e spa�� jeszcze w d�
{
  bool result=true;// z g�ry zak�adamy �e tak
  for (int i=0; i<4; i++)
  {  
     change_xy(alpha,i);
     if ((y<1)||(fields[x][y-1]>0))//je�eli nie ma jak
     result=false;//to si� zatrzymuje
     
     
  }
  return result;
}

void clear()//t� funkcj� wywo�ujemy tylko raz, aby unikn�� podstawienia na ni�ej wymienione zmienne losowych warto�ci
{
     
     
      for (int x=0; x<10; x++) 
      {
          for (int y=0; y<20; y++) 
          {
              fields[x][y] = false;
          }
      }
      
  startTime = GetTickCount();
  currentBrickShape = rand() % 7;//losujemy kszta�ty
  nextBrickShape = rand() % 7;
  currentBrickColor = (rand() % 8) + 1;//i kolory
  nextBrickColor = (rand() % 8) + 1;
  
}

void rotate()//obracamy klockiem
{
     if(brick_correct(brickPosX,brickPosY,brickShapes[currentBrickShape],alpha+1,0,true,fields))//oczywi�cie je�li mo�emy to zrobi�
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
 
void get_down()//zni�amy na sam d�
{   
         while(brick_can_fall(brickPosX, brickPosY, brickShapes[currentBrickShape],alpha,fields))
         brickPosY--;
             
}

void go_left()//ruszamy w lewo
{
     if (brick_correct(brickPosX,brickPosY,brickShapes[currentBrickShape],alpha,1,true,fields))  //je�li mo�emy, to przesuwamy w lewo          
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

void draw_next()//rysujemy nast�pnego klocka
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

int DrawGlScene()//g��wna funkcja
{
    

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  
  long currentTime = GetTickCount()-startTime;//obliczamy czas
  
  float orgin_x=(7-(14*FIELD_WIDTH))/2;
  float orgin_y=(10-(20*FIELD_WIDTH))/2;

  
  if (currentTime - last_fall>fall_time)//sprawdzamy czy nie za wcze�nie za ruch
  {
      last_fall = currentTime;
      
      if (brick_can_fall(brickPosX, brickPosY, brickShapes[currentBrickShape],alpha,fields))//je�li mo�e spada�, niech klocek dalej spada
      {
         brickPosY--;
         
      }
      else //je�eli ju� nie mo�e, zapisujemy jego kszta�t do tablicy
      {   
          for (int i=0;i<4;i++)
           {
              change_xy(alpha,i);//uwzgl�dniaj�c obr�t
                   
              switch (currentBrickColor)//pod postaci� int, dzi�ki kt�rym mo�emy narysowa� kolory
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
          nextBrickShape = rand() % 7;//i losujemy nast�pn�
          currentBrickColor = nextBrickColor;
          nextBrickColor = (rand() % 8) + 1;
          
          brickPosY=17;
          brickPosX=7;
          alpha=0;
          
          
            int points = check_line(fields);//sprawdzamy czy uda�o si� zape�ni� kt�r�� z li�         
            score+=10*points;//je�li tak to dodajemy punkty
            level=(score/100)+1;//obliczamy na kt�rym jeste�my poziomie (co 100 punkt�w level++)
            fall_time=500*pow(0.7,level);//zwi�kszamy szybko��
                 
                 
                 
          if(!brick_can_fall(brickPosX, brickPosY, brickShapes[currentBrickShape],alpha,fields))//je�eli nie mo�na zrzuci� nast�pnego klocka, koniec gry
          game_over=true;
     }
  }
  
  
  for (int i=0; i<14; i++) //rysujemy plansz� uwzgl�dniaj�c szeroko�� ekranu
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
  
  
  for (int i=0; i<4; i++)//ustalamy wsp�rz�dne klocka
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
 
 if (game_over)//sprawdzamy czy mo�na dalej gra�
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

