#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"


Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
Teren viditelnyTeren;
Teren objavenyTeren;
vector<Prikaz> prikazy;


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  // (sem patri vas kod)

  // hlupy klient si vyrobi Teren (cize 2D pole) kde si pamata co objavil
  objavenyTeren.vyprazdni(mapa.w, mapa.h, MAPA_NEVIEM);
}


// pomocna funkcia co ked uz vieme kam ten manik chce ist tak ho tam posle
static void chodKuMiestu(const Manik &m, Bod ciel) {
  Teren vzdialenost;
  prehladajBfs(objavenyTeren, ciel, vzdialenost);
  for (int d = 0; d < 4; d++) {
    Bod n(m.x + DX[d], m.y + DY[d]);
    if (priechodne(objavenyTeren.get(n)) &&
        vzdialenost.get(n) < vzdialenost.get(m.pozicia())) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_CHOD, n));
      break;
    }
  }
}


void coRobiKovac(const Manik &m) {
  // hlupy klient proste furt stavia banikov kolko moze...
  int d = rand() % 4;
  prikazy.push_back(Prikaz(m.id, PRIKAZ_KUJ, m.x + DX[d], m.y + DY[d], MANIK_BANIK));
}


void coRobiBanik(const Manik &m) {
  // najprv si zistim kde je kovac.
  int kovacX = -1, kovacY = -1;
  FOREACH(it, stav.manici) {
    if (it->ktorehoHraca == 0 && it->typ == MANIK_KOVAC) {
      kovacX = it->x; kovacY = it->y;
    }
  }

  for (int d = 0; d < 4; d++) {
    int nx = m.x + DX[d], ny = m.y + DY[d];
    // ak som hned vedla zlata alebo zeleza, tazim.
    if (objavenyTeren.get(nx, ny) == MAPA_ZLATO || objavenyTeren.get(nx, ny) == MAPA_ZELEZO) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, nx, ny));
      return;
    }
    // ak som hned vedla kovaca a mam mu co dat, dam mu to.
    if (nx == kovacX && ny == kovacY && m.zlato) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZLATO, nx, ny, m.zlato));
      return;
    }
    if (nx == kovacX && ny == kovacY && m.zelezo) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_DAJ_ZELEZO, nx, ny, m.zelezo));
      return;
    }
  }

  // ak som uz vytazil vela surovin, idem nazad za kovacom.
  if ((m.zlato >= 30 || m.zelezo >= 30) && kovacX != -1) {
    chodKuMiestu(m, Bod(kovacX, kovacY));
    return;
  }

  // ak vidime nejake zlato alebo zelezo, idem k nemu.
  Teren vzdialenost;
  prehladajBfs(objavenyTeren, m.pozicia(), vzdialenost);
  Bod bestp(-1, -1); int bestdist = mapa.w * mapa.h;
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if ((objavenyTeren.get(x, y) == MAPA_ZLATO || objavenyTeren.get(x, y) == MAPA_ZELEZO) && vzdialenost.get(x, y) < bestdist) {
      bestp = Bod(x, y); bestdist = vzdialenost.get(x, y);
    }
  }
  if (bestp.x != -1) {
    chodKuMiestu(m, bestp);
    return;
  }

  // ak nie, tak idem za najblizsim sutrom a snad niekde nieco najdem...
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (objavenyTeren.get(x, y) == MAPA_SUTER && vzdialenost.get(x, y) < bestdist) {
      bestp = Bod(x, y); bestdist = vzdialenost.get(x, y);
    }
  }
  if (bestp.x != -1) {
    // ked uz pri tom najblizsom sutri stojim tak ho vykopem
    if (abs(bestp.x - m.x) + abs(bestp.y - m.y) == 1) {
      prikazy.push_back(Prikaz(m.id, PRIKAZ_UTOC, bestp));
      return;
    }
    // inak sa k nemu priblizim
    chodKuMiestu(m, bestp);
    return;
  }
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat,
// co tato funkcia rozhodne pomocou: prikazy.push_back(Prikaz(...));
void zistiTah() {
  // (sem patri vas kod)

  fprintf(stderr, "zistiTah zacina %d\n", stav.cas);

  // zapamatame si teren co vidime a doteraz sme nevideli
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (viditelnyTeren.get(x, y) != MAPA_NEVIEM) {
      objavenyTeren.set(x, y, viditelnyTeren.get(x, y));
    }
  }

  // kazdemu nasmu manikovi povieme co ma robit (na to mame pomocne funkcie)
  FOREACH(it, stav.manici) {
    if (it->ktorehoHraca != 0) continue;
    switch (it->typ) {
      case MANIK_KOVAC:
        coRobiKovac(*it);
        break;

      case MANIK_BANIK:
        coRobiBanik(*it);
        break;
    }
  }

  fprintf(stderr, "prikazov %d\n", (int)prikazy.size());
}


int main() {
  // v tejto funkcii su vseobecne veci, nemusite ju menit (ale mozte).

  unsigned int seed = time(NULL) * getpid();
  srand(seed);

  nacitaj(cin, mapa);
  fprintf(stderr, "START pid=%d, seed=%u\n", getpid(), seed);
  inicializuj();

  while (cin.good()) {
    vector<int> zakodovanyTeren;
    nacitaj(cin, zakodovanyTeren);
    dekodujViditelnyTeren(zakodovanyTeren, viditelnyTeren);
    nacitaj(cin, stav);
    prikazy.clear();
    zistiTah();
    uloz(cout, prikazy);
    cout << ".\n" << flush;   // bodka a flush = koniec odpovede
  }

  return 0;
}

