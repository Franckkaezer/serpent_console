
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>

// Structure pour un segment de serpent
typedef struct Segment {
    int x, y;
    struct Segment *suivant;
} Segment;

// Fonction pour dessiner le cadre
void dessinerCadreTerminal(int hauteurAZ, int largeurAZ) {
    for (int i = 0; i < hauteurAZ; i++) {
        for (int j = 0; j < largeurAZ; j++) {
            if (i == 0 || i == hauteurAZ - 1 || j == 0 || j == largeurAZ - 1) {
                printf("*");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
}

// Positionne le curseur pour afficher un point
void placerPoint(int x, int y, char symbole, int couleur) {
    // Changer la couleur en fonction de la valeur de `couleur`
    if (couleur == 1) {
        printf("\033[31m"); // Rouge pour le point
    } else if (couleur == 2) {
        printf("\033[34m"); // Bleu pour le serpent
    }

    printf("\033[%d;%dH", y + 1, x + 1); // Positionne le curseur
    printf("%c", symbole);

    // Reinitialiser la couleur
    printf("\033[0m");
    fflush(stdout);
}

// Ajoute un nouveau segment a la fin du serpent
void ajouterSegment(Segment **tete, int x, int y) {
    Segment *nouveau = (Segment *)malloc(sizeof(Segment));
    nouveau->x = x;
    nouveau->y = y;
    nouveau->suivant = NULL;

    if (*tete == NULL) {
        *tete = nouveau;
    } else {
        Segment *temp = *tete;
        while (temp->suivant) temp = temp->suivant;
        temp->suivant = nouveau;
    }
}

// Genère une position aleatoire pour le point dans le cadre
void genererPositionPoint(int *pointX, int *pointY, int largeurAZ, int hauteurAZ) {
    *pointX = rand() % (largeurAZ - 2) + 1;
    *pointY = rand() % (hauteurAZ - 2) + 1;
}

// Verifie si le serpent se mord ou touche les bords
int verifierCollision(Segment *serpent, int largeurAZ, int hauteurAZ) {
    // Verifie si la tête touche les bords
    if (serpent->x <= 0 || serpent->x >= largeurAZ - 1 || serpent->y <= 0 || serpent->y >= hauteurAZ - 1) {
        return 1;  // Collision avec les bords
    }
    // Verifie si la tête se mord
    Segment *temp = serpent->suivant;
    while (temp) {
        if (temp->x == serpent->x && temp->y == serpent->y) {
            return 1;  // Collision avec le corps
        }
        temp = temp->suivant;
    }
    return 0;
}

// Fonction pour lire une touche sans attendre l'appui sur Entree
int getKeyPress() {
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Mode non canonique et pas d'echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK); // Non-bloquant
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, 0); // Bloquant
    return ch;
}

// Fonction pour afficher le score
void afficherScore(int score, int largeurAZ) {
    // Affiche le score dans le coin superieur droit
    printf("\033[1;1HScore: %d", score);
    fflush(stdout);
}

// Fonction pour ajuster la vitesse du jeu en fonction du score
int ajusterVitesse(int score) {
    // La vitesse diminue a mesure que le score augmente (plus bas = plus rapide)
    int vitesse = 250000 - (score * 5000);  // Ajustez la vitesse ici
    if (vitesse < 50000) {
        vitesse = 50000; // Limite la vitesse minimale
    }
    return vitesse;
}

int main() {
    int hauteurAZ = 20, largeurAZ = 50;
    int posXAZ = largeurAZ / 2, posYAZ = hauteurAZ / 2;
    int pointX, pointY;

    int directionXAZ = 1, directionYAZ = 0;
    Segment *serpent = NULL;
    ajouterSegment(&serpent, posXAZ, posYAZ);

    srand(time(NULL));
    genererPositionPoint(&pointX, &pointY, largeurAZ, hauteurAZ);

    system("clear");
    dessinerCadreTerminal(hauteurAZ, largeurAZ);

    int score = 0;  // Initialisation du score
    int pause = 0;  // Variable pour la gestion de la pause

    while (1) {
        // Efface l'ecran
        system("clear");
        dessinerCadreTerminal(hauteurAZ, largeurAZ);

        // Affiche le score
        afficherScore(score, largeurAZ);

        // Affiche le point en rouge
        placerPoint(pointX, pointY, 'O', 1);

        // Lecture de la touche
        int key = getKeyPress();

        if (key == 'w' && directionYAZ == 0) { directionXAZ = 0; directionYAZ = -1; } // Flèche haut
        else if (key == 's' && directionYAZ == 0) { directionXAZ = 0; directionYAZ = 1; } // Flèche bas
        else if (key == 'a' && directionXAZ == 0) { directionXAZ = -1; directionYAZ = 0; } // Flèche gauche
        else if (key == 'd' && directionXAZ == 0) { directionXAZ = 1; directionYAZ = 0; } // Flèche droite
        else if (key == ' ') { pause = !pause; }  // Espace pour pause

        if (!pause) {
            // Deplace la tête du serpent
            posXAZ += directionXAZ;
            posYAZ += directionYAZ;

            // Verifie la collision avec le point
            if (posXAZ == pointX && posYAZ == pointY) {
                ajouterSegment(&serpent, posXAZ, posYAZ);
                genererPositionPoint(&pointX, &pointY, largeurAZ, hauteurAZ);
                score++;  // Augmente le score a chaque point mange
            }

            // Met a jour la position des segments du serpent
            Segment *temp = serpent;
            int precedentX = serpent->x, precedentY = serpent->y;
            temp = temp->suivant;

            while (temp) {
                int tmpX = temp->x, tmpY = temp->y;
                temp->x = precedentX;
                temp->y = precedentY;
                precedentX = tmpX;
                precedentY = tmpY;
                temp = temp->suivant;
            }
            serpent->x = posXAZ;
            serpent->y = posYAZ;

            // Affiche le serpent en bleu
            temp = serpent;
            while (temp) {
                placerPoint(temp->x, temp->y, 'X', 2);
                temp = temp->suivant;
            }

            // Verifie si le serpent se mord ou touche les bords
            if (verifierCollision(serpent, largeurAZ, hauteurAZ)) {
                printf("\n\tGame Over! Vous avez perdu.\n");
                break;  // Fin du jeu si collision detectee
            }
        } else {
            // Affiche "Pause" pendant la pause
            placerPoint(largeurAZ / 2 - 2, hauteurAZ / 2, 'P', 2);
            placerPoint(largeurAZ / 2 + 3, hauteurAZ / 2, 'A', 2);
            placerPoint(largeurAZ / 2 + 6, hauteurAZ / 2, 'U', 2);
            placerPoint(largeurAZ / 2 + 9, hauteurAZ / 2, 'S', 2);
            placerPoint(largeurAZ / 2 + 12, hauteurAZ / 2, 'E', 2);
        }

        // Ajuste la vitesse en fonction du score
        usleep(ajusterVitesse(score));  // Pause ajustee par le score
    }

    // Liberer la memoire du serpent
    while (serpent) {
        Segment *temp = serpent;
        serpent = serpent->suivant;
        free(temp);
    }

    return 0;
}

