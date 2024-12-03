#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <curses.h>


#define MAX_TAILLE 20   //taille max du serpent
#define PAUSE 200000    // Définie la durée de la pause en microsecondes
#define X_START 40      // position initiale du serpent sur l'axe des x
#define Y_START 20      // Position initiale du serpent sur l'axe des y
#define X_PLATEAU 80    // dimension x du plateau
#define Y_PLATEAU 40    // dimension y du plateau
#define DIMENSION_PAVE 5// dimension des pavés
#define ESPACEMENT_PAVE 2    // espacement minimum entre les pavés et le bord du plateau 
//(0 fusionne le bord du pavé avec le bord du plateau )
#define ESPACEMENT_POMME 5  //espacement minimum entre le bord du plateau et la pomme
//(0 fusionne la pomme avec le bord du plateau )
#define NB_PAVES 4      // Nombre de pavés sur le plateau
#define CORPS 'X'       // Caractère de la tête
#define TETE 'O'        // Caractère du corps
#define STOP 'a'        // Caractère d'arrêt du programme
#define UP 'z'          // Caractère de direction du serpent vers le haut
#define DOWN 's'        // Caractère de direction du serpent vers le bas
#define LEFT 'q'        // Caractère de direction du serpent vers la gauche
#define RIGHT 'd'       // Caractère de direction du serpent verand() % ( X_POMME_MAX-X_POMME_MIN ) + rs la droite
#define BORDURE '#'     // Caractère de la bordure du plateau
#define VIDE ' '        // Caractère d'espace vide dans le plateau
#define POMME '6'

const int AUTHORIZED_CHAR[5] = { UP, DOWN, LEFT, RIGHT, STOP};

const int X_PAVE_MIN = ESPACEMENT_PAVE;                              // position X minimum des pavés
const int X_PAVE_MAX = X_PLATEAU - DIMENSION_PAVE - ESPACEMENT_PAVE; // position X maximum des pavés
const int Y_PAVE_MIN = ESPACEMENT_PAVE;                              // position y minimum des pavés
const int Y_PAVE_MAX = Y_PLATEAU - DIMENSION_PAVE - ESPACEMENT_PAVE; // position y maximum des pavés

const int X_POMME_MIN = ESPACEMENT_POMME;
const int X_POMME_MAX = X_PLATEAU - ESPACEMENT_POMME;
const int Y_POMME_MIN = ESPACEMENT_POMME;
const int Y_POMME_MAX = Y_PLATEAU - ESPACEMENT_POMME;

int TAILLE = 10; // Taille du serpent

typedef int t_plateau[Y_PLATEAU][X_PLATEAU];

/*
note : j'ai fait le choix ne pas initialiser de constante
pour définir la dimension du tableau de positions, car il est 
évident que celle-ci ne dépassera pas 2, puisque la console est elle-même
en 2 dimensions

note 2: il se peut que le serpent ne disparaisse pas lorsqu'il quitte le terminal
sur la droite ou vers le bas, c'est un bug lié au terminal, auquel je ne peux faire
grand chose, il faudrait récupérer les dimensions du terminal en temps réel et calculer à chaque
itération si le serpend quitte le terminal ou non, mais cela serait très fastidieux
*/



//Fonction de pause dans le programme
void c_sleep(float t);
//Désactivation de l'écoute des touches clavier
void disableEcho();
//Activation de l'écoute des touches clavier
void enableEcho();
//Vérification si une touche est pressée
int kbhit();

//Déplacement du curseur dans le terminal
void gotoXY(int x, int y);
//Vérification de la validité d'un caractère
int authorizedChar(int kbh);
//Vérification de la validité de la direction du serpent
int verifierTouche(int kbh, int new_char);

//Affichage d'un caractère à une position donnée
void afficher(int x, int y, char c);
//Effacement de l'entièreté du serpent
void effacerTout(int coord[2][MAX_TAILLE]);
//Effacement d'un caractère à une position donnée
void effacer(int x, int y);

//Ecriture du serpent dans le terminal
void dessinerSerpent(int laPosition[2][MAX_TAILLE]);
//Mise à jour des positions du serpent
void progresser(int laPosition[2][MAX_TAILLE], int kb_char, bool *collision, bool*, t_plateau t);
//Initialisation de la liste des positions du serpent
void faireListePosition(int laPosition[2][MAX_TAILLE], int x, int y);

//Création du plateau de jeu
void initPlateau(t_plateau t);
//Création des pavés
void creerPave(t_plateau t);
//Affichage du plateau
void dessinerPlateau(t_plateau t);

//création d'une pomme
void ajouterPomme(t_plateau t, int[2][MAX_TAILLE]);



int main(){

    //initialisation des variables
    srand(time(NULL));
    int x = X_START;
    int y = Y_START;
    int keyboard_char = RIGHT;
    int positionTab[2][MAX_TAILLE];
    bool collision = false;
    bool manger_pomme = false;
    float coef_vitesse = 1;
    t_plateau t;
    

    //appel des fonction de préparation
    system("clear");

    faireListePosition(positionTab, x, y);
    initPlateau(t);
    dessinerPlateau(t);
    ajouterPomme(t, positionTab);
    disableEcho();


    //déroulement du programme
    
    while(keyboard_char != STOP && collision != true)
    {
        dessinerSerpent(positionTab);

        if (kbhit())
        {
            keyboard_char = verifierTouche(keyboard_char, getchar());
        }

        effacerTout(positionTab);
        progresser(positionTab, keyboard_char, &collision, &manger_pomme, t);
        
        if(manger_pomme == true){
            ajouterPomme(t, positionTab);
            TAILLE += 1;
            coef_vitesse += 0.1;
            manger_pomme = false;

        }

        
        if (TAILLE == MAX_TAILLE){
           gotoXY(X_PAVE_MAX /2,Y_PAVE_MAX  + 10); // position de la phrase de victoire absolue par rapport au plateau
           printf("vous avez gagné !");
           keyboard_char = STOP;
        }
        

        c_sleep(PAUSE/coef_vitesse);

    }
    enableEcho();
     
    return 0;
}


void c_sleep(float t){
    /*
    Réalise une pause dans le déroulement du programme

    ------
    paramètres 
    int t : Durée de la pause en microsecondes
    */
    int dt = clock();
    while (clock() < t + dt){}
}


void disableEcho() {
    /*
    Désactive l'écoute des caractères saisis par l'utilisateur dans le terminal
    */
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Desactiver le flag ECHO
    tty.c_lflag &= ~ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}


void enableEcho() {
    /*
    Réactive l'écoute des caractères saisis par l'utilisateur dans le terminal
    */
    struct termios tty;

    // Obtenir les attributs du terminal
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    // Reactiver le flag ECHO
    tty.c_lflag |= ECHO;

    // Appliquer les nouvelles configurations
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}


int kbhit(){
    /*
    Vérifie si une touche est pressée

    ------
    Retour

    int : 1 si une touche est pressée, 0 sinon
    */
	int unCaractere=0;
	struct termios oldt, newt;
	int ch;
	int oldf;

	// mettre le terminal en mode non bloquant
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
	ch = getchar();

	// restaurer le mode du terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
 
	if(ch != EOF){
		ungetc(ch, stdin);
		unCaractere=1;
	} 
	return unCaractere;
}


void gotoXY(int x, int y) { 
    /*
    Déplace le curseur à la position (x,y)

    ------
    paramètres
    int x : Position x souhaitée pour le curseur
    int y : Position y souhaitée pour le curseur
    */
    printf("\033[%d;%df", y, x);
}


void afficher(int x, int y, char c){
    /*
    Affiche une caractère passée en paramètre à la position (x,y)

    ------
    paramètres
    int x : Position x souhaitée pour le caractère
    int y : Position y souhaitée pour le caractère
    char c : Caractère à afficher
    */
    gotoXY(x, y);
    printf("%c", c);
}


void effacer(int x, int y){
    /*
    Efface ( remplace par un espace ) le caractère à la position (x,y)

    ------
    paramètres
    int x : Position x du caractère
    int y : Position y du caractère
    */

    if (x > 0 && y > 0)
    {
        afficher(x, y, VIDE); 
    } 
}


void dessinerSerpent(int laPosition[2][MAX_TAILLE]){
    /*
    Affiche le serpent à partir de son tableau de positions
    
    ------
    paramètres

    int laPosition[2][MAX_TAILLE] : Tableau à 2 dimensions contenant les positions x du serpent 
    dans la première dimension, et les positions respectives y dans la deuxième.
    */
    int x, y;
    char c = TETE;

    for (int i = 0; i < TAILLE; i++)
    {   
        x = laPosition [0][i];
        y = laPosition [1][i];
        i > 0 ? c = CORPS : 0;

        if (x > 0 && y > 0)
        {
            afficher(x, y, c); 
        }    
    }   
}


void progresser(int laPosition[2][MAX_TAILLE], int kb_char, bool *collision, bool *manger, t_plateau t ){
    /*
    Calcule les position suivantes du serpent à partir de son tableau de positions
    
    ------
    paramètres
    int laPosition[2][MAX_TAILLE] : Tableau à 2 dimensions contenant les positions x du serpent 
    dans la première dimension, et les positions respectives y dans la deuxième.

    int kb_char : 

    t_plateau t : 

    bool *collision
    */
    
    for (int i = TAILLE - 1; i > 0; i--)
    {
        laPosition[0][i] = laPosition[0][i-1];
        laPosition[1][i] = laPosition[1][i-1];
    }

    switch (kb_char)
        {

        case UP:
            if (laPosition[1][0] - 1 < 1){
                laPosition[1][0] = Y_PLATEAU;
            } else {
                laPosition[1][0] = laPosition[1][0] - 1;
            }
            break;


        case LEFT:
            if (laPosition[0][0] - 1 < 1){
                laPosition[0][0] = X_PLATEAU;
            } else {
                laPosition[0][0] = laPosition[0][0] - 1;
            }
            break;


        case DOWN:
            if (laPosition[1][0] + 1 > Y_PLATEAU ){
                laPosition[1][0] = 1;
            } else {
                laPosition[1][0] = laPosition[1][0] + 1;
            }
            break;


        case RIGHT:
            if (laPosition[0][0] + 1 > X_PLATEAU ){
                    laPosition[0][0] = 1;
                } else {
                    laPosition[0][0] = laPosition[0][0] + 1;
                }
            break;
        
        default:
            break;
    }

    if (t[laPosition[1][0]-1][laPosition[0][0]-1] == BORDURE){
        *collision = true;
    } 
    if (t[laPosition[1][0] - 1][laPosition[0][0]-1] == POMME){

        *manger = true;

        t[laPosition[1][0]][laPosition[0][0]] = VIDE;
    }

    for(int i = TAILLE -1; i > 0 ; i--){ // on ne prend pas le dernier élémenr car c'est l'élément comparé, le comparer à lui-même serait absurde
        
        if ( laPosition[1][0] == laPosition[1][i] && laPosition[0][0] == laPosition[0][i]){
            *collision = true;
        }
    }
}


void faireListePosition(int laPosition[2][MAX_TAILLE], int x, int y){
    /*
    Initialise les positions du serpent dans le tableau des positions
    
    ------
    paramètres
    int laPosition[2][MAX_TAILLE] : Tableau de dimensions 2*MAX_TAILLE contenant les positions x du serpent 
    dans le premier sous-tableau, et les positions respectives y dans le deuxième.

    int x : Position x de la tête du serpent
    int y : Position y de la tête du serpent
    */
    for (int i = 0; i < TAILLE; i++)
    {
        laPosition[0][i] = x - i ;
        laPosition[1][i] = y;
    }   
}


void effacerTout(int coord[2][MAX_TAILLE]){
    /*
    Efface toutes les positions du serpent dans le tableau des positions
    
    ------
    paramètres
    int coord[2][MAX_TAILLE] : Tableau de dimensions 2*MAX_TAILLE contenant les positions x du serpent
    dans le premier sous-tableau, et les positions respectives y dans le deuxième.
    */
    for( int i = 0; i < TAILLE; i ++){
        effacer(coord[0][i], coord[1][i]);
    }
}


int authorizedChar(int kbh){
    /*
    Vérifie si la touche pressée est autorisée, c'est à dire comprise dans ['z', 'q','s', 'd', 'a']
    
    ------
    paramètres
    int kbh : Touche pressée par l'utilisateur

    ------
    retour

    int : 1 si la touche pressée est autorisée, 0 sinon.
    */

    for(int i = 0; i < sizeof(AUTHORIZED_CHAR)/4; i++){
        if(kbh == AUTHORIZED_CHAR[i]){
            return 1;
        }
    }
    return 0;
}


int verifierTouche(int kbh, int new_char){
    /*
    Vérifie et retourne la nouvelle touche pressée si elle est valide,
    retourne sinon la valeur actuelle de keyboard_char (kbh).
    
    ------
    paramètres

    int kbh : Valeur actuelle de keyboard_char
    int new_char : Nouvelle touche pressée par l'utilisateur

    ------
    retour

    int : Valeur de new_char si la touche pressée est valide, sinon la valeur de kbh.
    */
    int res = new_char;
    if( !authorizedChar(new_char) || (kbh + new_char ==  UP + DOWN) || (kbh + new_char == LEFT + RIGHT) ) 
    {
        res = kbh;
    }

    return res;
}


void initPlateau(t_plateau t){
    /*
    Initialise le plateau de jeu
    
    ------
    paramètres :

    t_plateau t : tableau à deux dimensions contenant le plateau de jeu
    */
   int character = BORDURE;
    for (int i = 0; i < Y_PLATEAU; i++){
        if (i > 0 && i < Y_PLATEAU-1){
                character = VIDE;
            } else {
                character =  BORDURE;
            }

        for (int y = 0; y < X_PLATEAU; y++){

            t[i][y] = character;
        }
        
        t[i][0] = BORDURE;
        t[i][X_PLATEAU-1] = BORDURE;
   }
   for (int i = 0; i < NB_PAVES; i++)
   {
        creerPave(t);
   }

   t[0][X_PLATEAU/2-1] = VIDE;
   t[Y_PLATEAU-1][X_PLATEAU/2-1] = VIDE;
   t[Y_PLATEAU/2 -1][0] = VIDE;
   t[Y_PLATEAU/2 -1][X_PLATEAU-1] = VIDE;
   
}


void creerPave(t_plateau t){
        /*
    crée un pavé de taille DIMENSION_PAVE

    ------
    paramètres :
    
    t_plateau t : tableau à deux dimensions contenant le plateau de jeu
    */
    int x, y;
    x = rand() % (X_PAVE_MAX-X_PAVE_MIN +1) + X_PAVE_MIN;
    y = rand() % (Y_PAVE_MAX-Y_PAVE_MIN +1) + Y_PAVE_MIN;
    if ((y > (Y_START - DIMENSION_PAVE - ESPACEMENT_PAVE) && y <= Y_START) && (x <= (X_START + ESPACEMENT_PAVE ) && x >= (X_START - TAILLE - DIMENSION_PAVE)))
    {
        x += DIMENSION_PAVE + TAILLE + ESPACEMENT_PAVE;
    }
    
    for (int i = 0; i < DIMENSION_PAVE; i++){
        for (int k = 0; k < DIMENSION_PAVE; k++){
            t[y+i][x+k] = BORDURE;

        }
    }

    
}

void dessinerPlateau(t_plateau t){
    /*
    Affiche le plateau de jeu
    
    ------
    paramètres :

    t_plateau t : tableau à deux dimensions contenant le plateau de jeu
    */
    for (int i = 0; i < Y_PLATEAU; i++){
        for (int y = 0; y < X_PLATEAU; y++){
            afficher(y +1, i+1, t[i][y]);
        }
        printf("\n");
    }
}


void ajouterPomme(t_plateau t, int p_tab[2][MAX_TAILLE]){
    /*
    Ajoute une pomme sur le plateau de jeu et l'affiche dans le terminal
    
    ------
    paramètres :

    t_plateau t : tableau à deux dimensions contenant le plateau de jeu

    int p_tab[2][MAX_TAILLE] : Tableau de dimensions 2*MAX_TAILLE contenant les positions x du serpent
    dans le premier sous-tableau, et les positions respectives y dans le deuxième.
    */
    int x, y;
    int void_space = 0;
    int i =0;
    
    while (!void_space){
        
        x = rand() % ( X_POMME_MAX-X_POMME_MIN ) + X_POMME_MIN;
        y = rand() % ( Y_POMME_MAX-Y_POMME_MIN ) + Y_POMME_MIN;
        
        if (t[y-1][x-1] == VIDE){
            void_space = 1;
        }
        
        i = 0;
        while(i < TAILLE && void_space){
            if ((x == p_tab[0][i]-1) || (y == p_tab[1][i]-1) )
                void_space = 0;
            i++;
        }
    }
    
    t[y-1][x-1] = POMME;
    afficher(x, y, POMME);
}