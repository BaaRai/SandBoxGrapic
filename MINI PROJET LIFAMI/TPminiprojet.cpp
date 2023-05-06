// apps/LIFAMI/TPGameOfLife.cpp
#include <Grapic.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <time.h>
using namespace std;
using namespace grapic;
const int MAX = 100;
const int DIMW = 500;
const int VIDE = 0;
const int PLEIN = 1;
const int SAND=2;
const int WATER=3;
const int WALL=5;
const int ACID= 6;
const int PLANT=7;
const int GAS=8;
const float FRICTION = 0.8;
const float dt = 0.001;
const int MAX_PART = 100;

struct sandBox
{
    int grille[MAX][MAX];
    int dx, dy;
    Image quitter;
    Image vortex;
};

struct vec2
{
    float x, y;
};

struct Color
{
    int R;
    int V;
    int B;
};

struct particule
{
    vec2 p, v, f;
    Color c;
    float m;
};

struct World
{
    particule tab[MAX_PART];
    int indice;
};

vec2 makevec2(float x, float y)
{
    vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

vec2 makevec2Exp(float r, float thetarad)
{
    vec2 v;
    v.x = r * cos(thetarad);
    v.y = r * sin(thetarad);
    return v;
}
float toDegre(float radiant)
{
    return radiant * 180 / M_PI;
}
float toRadiant(float degre)
{
    return degre * M_PI / 180;
}

vec2 operator+(vec2 a, vec2 b)
{
    vec2 v;
    v = makevec2(a.x + b.x, a.y + b.y);
    return v;
}

vec2 operator-(vec2 a, vec2 b)
{
    vec2 v;
    v.x = a.x - b.x;
    v.y = a.y - b.y;
    return v;
}
vec2 operator*(float lambda, vec2 p)
{
    vec2 v;
    v.x = lambda * p.x;
    v.y = lambda * p.y;
    return v;
}
vec2 operator*(vec2 p, float lambda)
{
    return makevec2(lambda * p.x, lambda * p.y);
}

vec2 operator/(vec2 p, float lambda)
{
    return makevec2(p.x / lambda, p.y / lambda);
}

vec2 scale(vec2 p, float vx, float vy, float lambda)
{
    vec2 q, v;
    v = makevec2(vx, vy);
    q = (p - v) * lambda + v;
    return q;
}

vec2 operator*(vec2 a, vec2 b)
{
    vec2 v;
    v.x = a.x * b.x - a.y * b.y;
    v.y = a.x * b.y + a.y * b.x;
    return v;
}

vec2 translate(vec2 p, float dx, float dy)
{
    vec2 v = makevec2(dx, dy);
    return p + v;
}

vec2 Rotate(vec2 p, float vx, float vy, float thetaRad)
{
    vec2 v, q, rot;
    v = makevec2(vx, vy);
    rot = makevec2Exp(1, thetaRad);
    q = (p - v) * rot + v;
    return q;
}

Color operator*(Color C1, float f)
{
    Color C;
    C.R = C1.R * f;
    C.V = C1.V * f;
    C.B = C1.B * f;
    return C;
}
Color operator+(Color C1, Color C2)
{
    Color C;
    C.R = C1.R + C2.R;
    C.V = C1.V + C2.V;
    C.B = C1.B + C2.B;
    return C;
}

Color makeColor(float R, float B, float V)
{
    Color c;
    c.R = R;
    c.B = B;
    c.V = V;
    return c;
}

void Init(sandBox &sB)
{
    int i, j;
    sB.dx = 50;
    sB.dy = 50;
    for (i = 0; i < sB.dx; i++)
    {
        for (j = 0; j < sB.dy; j++)
        {
            sB.grille[i][j] = VIDE;
        }
    }
    //sB.vortex=image("data/vortex.png");
}




void rectangleCell(sandBox &sB, int xm, int ym, int xM, int yM, int type)
{
    for(int i=xm; i<=xM; i++)
    {
        for(int j=ym; j<=yM; j++)
        {
            sB.grille[i][j]=type;
        }
    }
}

void bordure (sandBox &sB)
{
    for(int i=0; i<sB.dx; i++)
    {
        for(int j=0; j<sB.dy; j++)
        {
            rectangleCell(sB,0,i,j,0,WALL);// bas
            rectangleCell(sB,sB.dx-1,i,j,sB.dy-1,WALL);// droite
            rectangleCell(sB,i,sB.dx-1,j,sB.dy-1,WALL); // haut
            rectangleCell(sB,i,0,0,j,WALL);   //gauche
        }
    }
}

void gestionSouris(sandBox &sB, int type)
{
    if (isMousePressed(SDL_BUTTON_LEFT))
    {
        int x, y;
        mousePos(x, y);
        int hauteur, largeur;
        largeur = DIMW / sB.dx;
        hauteur = DIMW / sB.dy;
        x = x / largeur;
        y = y / hauteur;
        vec2 souris=makevec2(x,y);
        if (sB.grille[x][y]==VIDE)
            sB.grille[x][y]=type;
    }

    if(isMousePressed(SDL_BUTTON_RIGHT))
    {
        int x, y;
        mousePos(x, y);
        int hauteur, largeur;
        largeur = DIMW / sB.dx;
        hauteur = DIMW / sB.dy;
        x = x / largeur;
        y = y / hauteur;
        vec2 souris=makevec2(x,y);
        if(sB.grille[x][y]==type)
            sB.grille[x][y] = VIDE;
    }
}

void reset(sandBox &sB)
{

    if(isKeyPressed('r'))
    {
        for (int i = 1; i < sB.dx-1; i++)
        {
            for (int j = 1; j <sB.dy-1; j++)
            {
                sB.grille[i][j]=VIDE;
            }
        }
    }
}

void sbUpdateSand(sandBox &sB, int i, int j)
{
    int centre = sB.grille[i][j];
    int droite = sB.grille[i+1][j];
    int gauche = sB.grille[i-1][j];
    int bas = sB.grille[i][j-1];
    int basGauche = sB.grille[i - 1][j - 1];
    int basDroite = sB.grille[i + 1][j - 1];
    bool verif[3][3];
    for(int k=0; k<3; k++)
    {
        for (int l=0 ; l<3; l++)
        {
            verif[k][l]=true;
        }
    }
    if (centre != VIDE && bas == VIDE)
    {
        sB.grille[i][j] = VIDE;
        sB.grille[i][j - 1] = SAND;
    }
    if(bas == WATER )
    {
        sB.grille[i][j]=WATER;
        sB.grille[i][j-1]=SAND;
    }
    else if (centre != VIDE && bas != VIDE)
    {
        if (bas ==WALL && gauche ==WALL)
        {
            sB.grille[i][j]=SAND;
            verif[0][0]=false;
        }
        if(droite== WALL && bas==WALL)
        {
            sB.grille[i][j]=SAND;
            verif[2][0]=false;
        }
        if (basGauche == VIDE && basDroite == VIDE)
        {
            int pileFace=irand(0,1);
            if (pileFace == 0)
            {
                if(verif[2][0]==true)
                {
                    // std::cout<<"ok"<<endl;
                    sB.grille[i + 1][j - 1] = SAND;
                    sB.grille[i][j] = VIDE;
                }
                //std::cout<<pileFace<<endl;
            }
            else
            {
                if(verif[0][0]==true)
                {
                    //std::cout<<"ok"<<endl;
                    sB.grille[i - 1][j - 1] = SAND;
                    sB.grille[i][j] = VIDE;
                }
                //std::cout<<pileFace<<endl;
            }

        }
        else if (basGauche != VIDE && basDroite == VIDE)
        {
            if(verif[2][0]==true)
            {
                //std::cout<<"ok"<<endl;
                sB.grille[i][j] = VIDE;
                sB.grille[i + 1][j - 1] = SAND;
            }
        }
        else if (basGauche == VIDE && basDroite != VIDE )
        {
            if(verif[0][0]==true)
            {
                // std::cout<<"ok"<<endl;
                sB.grille[i][j] = VIDE;
                sB.grille[i - 1][j - 1] = SAND;
            }
        }

    }
}
const int pileFace=irand(0,1);
void sbUpdateWater(sandBox &sB, int i, int j)
{
    int centre = sB.grille[i][j];
    int droite = sB.grille[i+1][j];
    int gauche = sB.grille[i-1][j];
    int bas = sB.grille[i][j-1];
    int basGauche = sB.grille[i - 1][j - 1];
    int basDroite = sB.grille[i + 1][j - 1];
    int haut = sB.grille[i][j+1];
    bool verif[3][3];
    for(int k=0; k<3; k++)
    {
        for (int l=0 ; l<3; l++)
        {
            verif[k][l]=true;
        }
    }
    if (centre != VIDE && bas == VIDE)
    {
        sB.grille[i][j] = VIDE;
        sB.grille[i][j - 1] = WATER;
        //cout<<"test0"<<endl;
    }
    if(bas==ACID)
    {
        sB.grille[i][j] = VIDE;
        sB.grille[i][j-1]=VIDE;
    }

    else if (centre != VIDE && bas != VIDE)
    {
        //cout<<"test3"<<endl;
        if (bas ==WALL && gauche ==WALL)
        {
            verif[0][0]=false;
            //cout<<"test4"<<endl;
        }
        if(droite== WALL && bas==WALL)
        {
            verif[2][0]=false;
            //cout<<"test5"<<endl;
        }

        if (basGauche == VIDE && basDroite == VIDE)
        {
            int pileFace=irand(0,1);
            if (pileFace == 0)
            {
                if(verif[2][0]==true)
                {
                    // std::cout<<"ok"<<endl;
                    sB.grille[i + 1][j - 1] = WATER;
                    sB.grille[i][j] = VIDE;
                }
                //std::cout<<pileFace<<endl;
            }
            else
            {
                if(verif[0][0]==true)
                {
                    //std::cout<<"ok"<<endl;
                    sB.grille[i - 1][j - 1] = WATER;
                    sB.grille[i][j] = VIDE;
                }
            }
        }
        else if (basGauche != VIDE && basDroite == VIDE)
        {
            if(verif[2][0]==true)
            {
                //std::cout<<"ok"<<endl;
                sB.grille[i + 1][j - 1] = WATER;
                sB.grille[i][j] = VIDE;

            }
        }
        else if (basGauche != VIDE && basDroite == VIDE )
        {
            if(verif[0][0]==true)
            {
                //std::cout<<"ok"<<endl;
                sB.grille[i][j] = VIDE;
                sB.grille[i + 1][j - 1] = WATER;
                }
        }
        else if (gauche ==  VIDE && droite ==  VIDE )
        {
            int test =irand(0,1);
            if (test == 0)
            {
                sB.grille[i-1][j]=WATER;
                sB.grille[i][j] = VIDE;
                //cout<<"test10"<<endl;
                //cout<<test<<endl;
            }
            else
            {
                sB.grille[i+1][j]=WATER;
                sB.grille[i][j] = VIDE;
                //cout<<test;
                //cout<<"test1"<<endl;
            }
            sB.grille[i][j] = VIDE;
        }
        else if (gauche !=VIDE && droite== VIDE)
        {
            sB.grille[i+1][j]=WATER;
            sB.grille[i][j]=VIDE;
        }
        else if(gauche==VIDE && droite!=VIDE)
        {
            sB.grille[i-1][j]=WATER;
            sB.grille[i][j]=VIDE;
        }
    }
}


void sbUpdateWall(sandBox &sB, int i, int j)
{
    int centre = sB.grille[i][j];
    if(centre==PLEIN)
    {
        sB.grille[i][j]=PLEIN;
    }
}

void sbUpdateAcid (sandBox &sB, int i, int j)
{
    int centre = sB.grille[i][j];
    int droite = sB.grille[i+1][j];
    int gauche = sB.grille[i-1][j];
    int bas = sB.grille[i][j-1];
    int basGauche = sB.grille[i - 1][j - 1];
    int basDroite = sB.grille[i + 1][j - 1];
    int haut = sB.grille[i][j+1];
    bool verif[3][3];
    for(int k=0; k<3; k++)
    {
        for (int l=0 ; l<3; l++)
        {
            verif[k][l]=true;
        }
    }
    if (centre != VIDE && bas == VIDE)
    {
        sB.grille[i][j] = VIDE;
        sB.grille[i][j - 1] = ACID;
    }
    /*if(haut==SAND)
    {
        INUTILE, CEST POUR MULTIPLIER !
        sB.grille[i][j+1]=ACID;
        sB.grille[i][j] = VIDE;
    }*/
    if(haut==SAND)
    {
        sB.grille[i][j+1]=VIDE;
        sB.grille[i][j] = ACID;
    }
    if(droite==SAND)
    {
        sB.grille[i+1][j]=VIDE;
        sB.grille[i][j] = ACID;
    }
    if(bas==SAND)
    {
         sB.grille[i][j-1]=VIDE;
         sB.grille[i][j] = ACID;
    }
    if (gauche==SAND)
    {
        sB.grille[i-1][j]=VIDE;
        sB.grille[i][j] = ACID;
    }
    if(bas==WATER)
    {
        sB.grille[i][j] = VIDE;
        sB.grille[i][j-1]=VIDE;
    }
    if(droite==PLANT)
    {
        sB.grille[i+1][j]=VIDE;
        sB.grille[i][j] = VIDE;
    }
    if(bas==PLANT)
    {
         sB.grille[i][j-1]=VIDE;
         sB.grille[i][j] = VIDE;
    }
    if (gauche==PLANT)
    {
        sB.grille[i-1][j]=VIDE;
        sB.grille[i][j] = VIDE;
    }
    if(haut==PLANT)
    {
        sB.grille[i][j+1]=VIDE;
        sB.grille[i][j] = ACID;
    }
    else if (centre != VIDE && bas != VIDE)
    {
        if (bas ==WALL && gauche ==WALL)
        {
            verif[0][0]=false;
        }
        if(droite== WALL && bas==WALL)
        {
            sB.grille[i][j]=ACID;
            verif[2][0]=false;
        }
        if (basGauche == VIDE && basDroite == VIDE)
        {
            int pileFace=irand(0,1);

            if (pileFace == 0)
            {
                if(verif[2][0]==true)
                {
                    // std::cout<<"ok"<<endl;
                    sB.grille[i + 1][j - 1] = ACID;
                    sB.grille[i][j] = VIDE;
                }
                //std::cout<<pileFace<<endl;
            }
            else
            {
                if(verif[0][0]==true)
                {
                    //std::cout<<"ok"<<endl;
                    sB.grille[i - 1][j - 1] = ACID;
                    sB.grille[i][j] = VIDE;
                }
                //std::cout<<pileFace<<endl;
            }

        }
        else if (basGauche != VIDE && basDroite == VIDE)
        {
            if(verif[2][0]==true)
            {
                //std::cout<<"ok"<<endl;
                sB.grille[i][j] = VIDE;
                sB.grille[i + 1][j - 1] = ACID;
            }
        }
        else if (basGauche == VIDE && basDroite != VIDE )
        {
            if(verif[0][0]==true)
            {
                std::cout<<"ok"<<endl;
                sB.grille[i][j] = VIDE;
                sB.grille[i - 1][j - 1] = ACID;
            }
        }
        else if (gauche ==  VIDE && droite ==  VIDE )
        {
            int pileFace=irand(0,1);
            if (pileFace == 0)
            {
                sB.grille[i+1][j]=ACID;
            }
            else
            {
                sB.grille[i-1][j]=ACID;
            }
            sB.grille[i][j] = VIDE;
        }
        else if (gauche !=VIDE && droite== VIDE)
        {
            sB.grille[i][j]=VIDE;
            sB.grille[i+1][j]=ACID;
        }
        else if(gauche==VIDE && droite!=VIDE )
        {
            sB.grille[i][j]=VIDE;
            sB.grille[i-1][j]=ACID;
        }
    }
}


const int aleagas=irand(0,8);

void sbUpdateGas(sandBox &sB, int i, int j)
{
    int hautGauche=sB.grille[i-1][j+1];
    int haut =sB.grille[i][j+1];
    int hautDroite = sB.grille[i + 1][j + 1];
    int gauche = sB.grille[i-1][j];
    int droite = sB.grille[i+1][j];
    int centre = sB.grille[i][j];
    int basGauche = sB.grille[i - 1][j - 1];
    int bas = sB.grille[i][j-1];
    int basDroite = sB.grille[i + 1][j - 1];
    bool verif[3][3];
    for(int k=0; k<3; k++)
    {
        for (int l=0 ; l<3; l++)
        {
            verif[k][l]=true;
        }
    }
    //if(c)

}



void sbUpdatePlant(sandBox &sB, int i, int j)
{
    int centre = sB.grille[i][j];
    int droite = sB.grille[i+1][j];
    int gauche = sB.grille[i-1][j];
    int bas = sB.grille[i][j-1];
    int basGauche = sB.grille[i - 1][j - 1];
    int basDroite = sB.grille[i + 1][j - 1];
    int haut = sB.grille[i][j+1];
    bool verif[3][3];
    for(int k=0; k<3; k++)
    {
        for (int l=0 ; l<3; l++)
        {
            verif[k][l]=true;
        }
    }
    if (centre != VIDE && bas == VIDE)
    {
        sB.grille[i][j] = VIDE;
        sB.grille[i][j - 1] = PLANT;
    }
    if(haut==WATER)
    {
        sB.grille[i][j+1]=PLANT;
        sB.grille[i][j] = VIDE;
    }
    if(bas==WATER)
    {
        sB.grille[i][j-1]=PLANT;
        sB.grille[i][j] = VIDE;
    }
    if(gauche==WATER)
    {
        sB.grille[i-1][j]=PLANT;
        sB.grille[i][j]= VIDE;
    }
}

void sbUpdate(sandBox &sB)
{
    for (int i = 0; i < sB.dx; i++)
    {
        for (int j = 0; j <sB.dy; j++)
        {
            if (i >= 1 && j >= 1 && i < sB.dx-1 && j < sB.dy-1) // si j>=1 et i>=0 les limites marche bien en bas sauf la cologne entiere a gauche bug si j>=0 et i>=0 cologne gauche bug encore mais passe en bas
            {
                if(sB.grille[i][j]==SAND)
                {
                    sbUpdateSand(sB, i, j);
                }
                else if(sB.grille[i][j]==WATER)
                {
                    sbUpdateWater(sB,i,j);
                }
                else if(sB.grille[i][j]==WALL)
                {
                    sbUpdateWall(sB,i,j);
                }
                else if(sB.grille[i][j]==GAS)
                {
                    sbUpdateGas(sB,i,j);
                }
                else if(sB.grille[i][j]==ACID)
                {
                    sbUpdateAcid(sB,i,j);
                }
                else if(sB.grille[i][j]==PLANT)
                {
                    sbUpdatePlant(sB,i,j);
                }

            }
        }
    }
}


void draw (sandBox &sB)
{
    int i, j;
    grid(0, 0, DIMW - 1, DIMW - 1, sB.dx, sB.dy);
    int tailleX = DIMW / sB.dx;
    int tailleY = DIMW / sB.dy;
    for (i = 0; i < sB.dx; i++)
    {
        for (j = 0; j < sB.dy; j++)
        {
            if (sB.grille[i][j]==SAND)
            {
                color(199, 176, 24);
                circleFill(i * tailleX + tailleX / 2, j * tailleY + tailleY / 2,tailleX / 2-2);
            }
            else if (sB.grille[i][j]==WALL)
            {
                color(220,28,220);
                rectangleFill(i*tailleX,j*tailleY,(i+1)*tailleX,(j+1)*tailleY);
            }
            else if(sB.grille[i][j]==WATER)
            {
                color(50, 0, 255);
                circleFill(i * tailleX + tailleX / 2, j * tailleY + tailleY / 2,tailleX / 2-2);
            }
            else if(sB.grille[i][j]==GAS)
            {
                color(242, 224, 174);
                circleFill(i * tailleX + tailleX / 2, j * tailleY + tailleY / 2,tailleX / 2-2);
            }
            else if (sB.grille[i][j]==ACID)
            {
                color(255, 255, 5);
                circleFill(i * tailleX + tailleX / 2, j * tailleY + tailleY / 2,tailleX / 2-2);
                //image_draw(sB.vortex,i * tailleX + tailleX / 2,j * tailleY + tailleY / 2,20,20);
            }
            else if(sB.grille[i][j]==PLANT)
            {
                color(24, 199, 42);
                circleFill(i * tailleX + tailleX / 2, j * tailleY + tailleY / 2,tailleX / 2-2);
            }

        }
    }
}

int main(int, char **)
{
    winInit("SandBox", 500,500);
    bool stop = false;
    backgroundColor(0,0,0);
    srand(time(NULL));
    Menu menu;
    menu_add(menu,"Sand");
    menu_add(menu,"Wall");
    menu_add(menu,"Water");
    menu_add(menu,"Acid");
    menu_add(menu,"Gaz");
    menu_add(menu,"Plant");
    sandBox sB;
    Init(sB);
    bordure(sB);
    int type=SAND;
    /*int choice;
    cout<<"0. quit"<<endl;
    cout<<"1. Play Game"<<endl;
    cin>>choice;
    switch(choice)
    {
    case 0:
        cout<<"bye"<<endl;
        break;
    case 1:*/
        while (!stop)
        {
            setKeyRepeatMode(true);
            winClear();
            sbUpdate(sB);
            draw(sB);
            reset(sB);
            switch (menu_select(menu) )
            {

            case 0:
                type=SAND;
                //delay(50);
                break;
            case 1:
                type=WALL;
                //delay(50);
                break;
            case 2:
                type=WATER;
                //delay(50);
                break;
            case 3:
                type=ACID;
                //delay(50);
                break;
            case 4:
                type=GAS;
                break;
            case 5:
                delay(5);
                type=PLANT;
                break;

            }
            gestionSouris(sB,type);
            menu_draw(menu,5,5,-1,-1);
            stop = winDisplay();
        /*}
        break;*/
    }
    winQuit();
    return 0;
}
