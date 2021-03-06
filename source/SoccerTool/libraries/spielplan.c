#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "spielplan.h"
#include "stoppuhr.h"

FILE *spielplan;
int spielzeit[4]; //Vorrunde, Halbfinale, Platz 3, Finale
char teams[2][30][10]; //Gruppe, Team (max. 30), Name
int anzahl_teams[2]; //Teams in Gruppen
char vorrunde[30][2][10]; //Spiele in der Vorrunde (max. 30)
int anzahl_spiele; //Spiele in der Vorrunde
int spiel[2]; //Index Spiele Feld 1 und Feld2, -1 = Halbfinale, -2 = Spiel um Platz 3, -3 = Finale, -4: = Unbelegt
int tore[2][2]; //Feld, Team
int anzahl_ergebnisse; //Anzahl der bereits vorliegenden Ergebnisse (abgeschlossene Spiele)
int ergebnisse[30][2]; //Ergebnisse (Tore). Index 1 = Spiel, Index 2 = Mannschaft wie in teams
int punkte[2][30]; //Punkte in Gruppe. Team (max. 30)
char spielfeld_leer[2] = "-\0";

void open_spielplan(void) {
  int modus=0, buffer_index, gruppe; //1=Beginn, 2=Spielzeit, 3=Gruppe 1, 4=Gruppe 2, 5=Vorrunde, 6=Ergebnisse
  char *buffer, data[3][15]; //Buffer

  spielplan = fopen("spielplan.soc", "a+");
  if (spielplan == NULL) {
    g_print("Spielplan kann nicht geoeffnet werden!\n");
    exit(1);
  }

  char line [128];
  while (fgets(line, sizeof(line), spielplan) != NULL) {
    line[strlen(line) - 1] = '\0'; //Zeilenweise auslesen
    buffer_index=0;

    if (!strcmp(line,"+++beginn+++") && modus==0) modus=1; //TODO Ausrufezeichen statt 0
    else if (!strcmp(line,"---spielzeit---") && modus==1) modus=2;  //TODO Ausrufezeichen statt 0
    else if (!strcmp(line,"---gruppe1---") && modus==2) { modus=3; anzahl_teams[0]=0; gruppe=0; }
    else if (!strcmp(line,"---gruppe2---") && modus==3) { modus=4; anzahl_teams[1]=0; gruppe=1; }
    else if (!strcmp(line,"---vorrunde---") && modus==4) { modus=5; anzahl_spiele=0; }
    else if (!strcmp(line,"---ergebnisse---") && modus==5) { modus=6; anzahl_ergebnisse=0; }
    else if (modus==2 || modus==5 || modus==6) {
      buffer = strtok (line,"-:");
      while (buffer != NULL) {
        strcpy(data[buffer_index], buffer);
        ++buffer_index;
        buffer = strtok (NULL, ":");
      }

      if (modus==2) { //Spielzeit einlesen
        if (!strcmp(data[0],"vorrunde")) spielzeit[0] = atoi(data[1]);
        else if (!strcmp(data[0],"halbfinale")) spielzeit[1] = atoi(data[1]);
        else if (!strcmp(data[0],"platz3")) spielzeit[2] = atoi(data[1]);
        else if (!strcmp(data[0],"finale")) spielzeit[3] = atoi(data[1]);
      } else if (modus==5) { //Vorrunde einlesen
        if (anzahl_spiele<30) {
          strcpy(vorrunde[anzahl_spiele][0], data[1]);
          strcpy(vorrunde[anzahl_spiele][1], data[2]);
          ++anzahl_spiele;
        } else {
          g_print("Fehler: Zu viele Mannschaften in Gruppe %d\n", gruppe);
          exit(0);
        }
      } else if (modus==6) { //Vorhandene Ergebnisse einlesen
        ergebnisse[anzahl_ergebnisse][0] = atoi(data[1]);
        ergebnisse[anzahl_ergebnisse][1] = atoi(data[2]);
        ++anzahl_ergebnisse;
      }
    } else if (modus==3 || modus==4) {
      if (anzahl_teams[gruppe]<30) {
        strcpy(teams[gruppe][anzahl_teams[gruppe]], line);
        ++anzahl_teams[gruppe];
      } else {
        g_print("Fehler: Zu viele Mannschaften in Gruppe %d\n", gruppe);
        exit(0);
      }
    }
  }
  if (modus!=6) {
    g_print("Fehler: Der Spielplan ist nicht wohlgeformt\n"); //TODO: Fehlermeldungen als alert
    exit(1);
  }

  spiel[0] = -2 + anzahl_ergebnisse;
  spiel[1] = -1 + anzahl_ergebnisse;
  naechstes_spiel_initialisieren();
}

void close_spielplan(void) {
  fclose(spielplan);
}

int get_spielzeit() {
  if (spiel[0] >= 0) //Vorrunde
    return spielzeit[0];
}

const char *get_team(int feld, int team) {
  if (((feld==0 && spiel[0]!=-4) || (feld==1 && spiel[1]!=-4)) && (team==0 || team==1))
    return vorrunde[spiel[feld]][team];
  else {
    return spielfeld_leer;
  }
}

void vorheriges_spiel(void) {
  tore[0][0] = 0;
  tore[0][1] = 0;
  tore[1][0] = 0;
  tore[1][1] = 0;

  if (spiel[0]-2 >= 0) {
    spiel[0]-=2;
    stoppuhr_stop(0);
    stoppuhr_set(0, spielzeit[0]);
  }
  if (spiel[1]-2 >= 1) {
    spiel[1]-=2;
    stoppuhr_stop(1);
    stoppuhr_set(1, spielzeit[0]);
  }
}

void halbfinale(void) {

}

void naechstes_spiel_initialisieren(void) {
  tore[0][0] = 0;
  tore[0][1] = 0;
  tore[1][0] = 0;
  tore[1][1] = 0;

  if (spiel[0]+2 < anzahl_spiele) {
    stoppuhr_stop(0);
    stoppuhr_set(0, spielzeit[0]);
    spiel[0]+=2;
  } else
    spiel[1]=-4;
  if (spiel[1]+2 < anzahl_spiele) {
    stoppuhr_stop(1);
    stoppuhr_set(1, spielzeit[0]);
    spiel[1]+=2;
  } else
    spiel[1]=-4;
}

void naechstes_spiel(void) {
  fprintf(spielplan, "%d-%d:%d\n", anzahl_ergebnisse++, tore[0][0], tore[0][1]); //Ergebnisse speichern
  fprintf(spielplan, "%d-%d:%d\n", anzahl_ergebnisse++, tore[1][0], tore[1][1]);
  fflush(spielplan);
  naechstes_spiel_initialisieren();
}

void tor_inkrementieren(int feld, int team) {
  if ((feld==0 || feld==1) && (team==0 || team==1))
    ++tore[feld][team];
  else
    g_print("Falsches Feld/Team");
}

void tor_dekrementieren(int feld, int team) {
  if ((feld==0 || feld==1) && (team==0 || team==1) && tore[feld][team]>0)
    --tore[feld][team];
  else
    g_print("Falsches Feld/Team oder nicht dekrementierbar");
}

int get_tore(int feld, int team) {
  if ((feld==0 || feld==1) && (team==0 || team==1))
    return tore[feld][team];
  else
    g_print("Falsches Feld/Team");
}
