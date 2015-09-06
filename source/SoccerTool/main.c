#include <gtk/gtk.h>
#include "windows/admin.h"
#include "windows/beamer.h"
#include "libraries/spielplan.h"
#include "libraries/stoppuhr.h"

int main(int argc, char **argv) {
  open_spielplan();
  close_spielplan();

  gtk_init(&argc, &argv);

  beamer_init(); //TODO: Swap
  admin_init(); //TODO: Swap
  stoppuhr_init();
  beamer_set_team(0,0,get_team(0,0));
  beamer_set_team(0,1,get_team(0,1));
  beamer_set_team(1,0,get_team(1,0));
  beamer_set_team(1,1,get_team(1,1));
  gtk_main();
  return 0;
}
