
#ifndef COMMON_H
#define COMMON_H

// vseobecne datove struktury a tak podobne

#include <vector>

#define FOREACH(it,c) for (typeof((c).begin()) it = (c).begin(); it != (c).end(); ++it)

#define MAPA_OKRAJ        0
#define MAPA_KAMEN        1
#define MAPA_HLINA        2
#define MAPA_BOMBA        3
#define MAPA_VOLNO        4
#define MAPA_START        5
#define MAPA_BONUS        6

#define priechodne(p) ((p) >= MAPA_VOLNO)

#define BONUS_SILA        0
#define BONUS_POCET       1
#define BONUS_STIT        2
#define BONUS_OHNOSTROJ   3
#define BONUS_VIANOCE     4

#define BONUS_POCET_TYPOV 5

#define SMER_NIKAM -1
#define SMER_HORE 0
#define SMER_VPRAVO 1
#define SMER_DOLE 2
#define SMER_VLAVO 3

struct Bod {
  int x, y;
  Bod() : x(0), y(0) {}
  Bod(int _x, int _y) : x(_x), y(_y) {}
  bool operator==(const Bod& b) const { return x == b.x && y == b.y; }
  bool operator!=(const Bod& b) const { return !(x == b.x && y == b.y); }
  bool operator<(const Bod& b) const { return y < b.y || (y == b.y && x < b.x); }
  Bod operator+(const Bod& b) const {return Bod(x + b.x, y + b.y); }
};

struct Bonus {
  int id;
  int x;
  int y;
  int typ;
  Bod pozicia() const { return Bod(x, y); }
};

struct Bomba {
  int id;
  int kto; // koho je tato bomba. -1, ak to je ohnostroj
  int x;
  int y;
  int timer; // o kolko kol vybuchne
  int sila;
  Bod pozicia() const { return Bod(x, y); }
};

struct Prikaz {
  int smer; // -1 az 3, podla DX, DY
  bool kladiem;
  Prikaz() : smer(SMER_NIKAM), kladiem(false){}
  Prikaz(int _smer, bool _kladiem)
      : smer(_smer), kladiem(_kladiem) {
  }
};


typedef Prikaz Odpoved;


struct Hrac {
  std::vector<int> mapovanie;   // klienti nevidia
  int skore;
  int x;
  int y;
  bool jeZivy;
  int pocetBomb; // pocet bomb daneho hraca na mape
  
  // veci, suvisiace s bonusmi
  int maxPocetBomb; // kolko moze mat bomb naraz, meni sa
  int silaBomb; // sila bomb daneho hraca, meni sa
  int maStit; // kolko kol este ma stit
  
  Bod pozicia() const { return Bod(x, y); }
};


struct Teren {
  std::vector<std::vector<int> > data;

  int h() const { return data.size(); }
  int w() const { return data.empty() ? 0 : data[0].size(); }
  int get(int x, int y) const {
    return (y >= 0 && y < (int)data.size() &&
            x >= 0 && x < (int)data[y].size() ? data[y][x] : MAPA_OKRAJ);
  }
  int get(Bod b) const { return get(b.x, b.y); }
  void set(int x, int y, int t) {
    if (y >= 0 && y < (int)data.size() &&
        x >= 0 && x < (int)data[y].size()) data[y][x] = t;
  }
  void set(Bod b, int t) { set(b.x, b.y, t); }
  void vyprazdni(int w, int h, int t) {
    data.resize(h);
    for (int y = 0; y < h; y++) data[y].assign(w, t);
  }
};


struct Stav {
  std::vector<Hrac> hraci;
  std::vector<Bonus> bonusy;
  std::vector<Bomba> bomby;
  Teren teren;
  int dalsiId;
  int cas;
};


struct Mapa {
  int pocetHracov;
  int w;
  int h;
  Teren pribliznyTeren;   // bonusy vznikaju priebezne, kamene zanikaju
};


#define FORMAT_VERSION 1

#endif

#ifdef reflection
// tieto udaje pouziva marshal.cpp aby vedel ako tie struktury ukladat a nacitavat

reflection(Bod);
  member(x);
  member(y);
end();

reflection(Bonus);
  member(x);
  member(y);
  member(typ);
end();

reflection(Bomba);
  member(kto);
  member(x);
  member(y);
  member(sila);
  member(timer);
end();

reflection(Prikaz);
  member(smer);
  member(kladiem);
end();

reflection(Hrac);
  member(skore);
  member(x);
  member(y);
  member(jeZivy);
  member(pocetBomb);
  member(maxPocetBomb);
  member(silaBomb);
  member(maStit);
end();

reflection(Teren);
  member(data);
end();

reflection(Stav);
  member(hraci);
  member(bonusy);
  member(bomby);
  member(teren);
  member(dalsiId);
  member(cas);
end();

reflection(Mapa);
  member(pocetHracov);
  member(w);
  member(h);
  member(pribliznyTeren);
end();

#endif
