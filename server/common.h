
#ifndef COMMON_H
#define COMMON_H

// vseobecne datove struktury a tak podobne

#include <vector>

#define FOREACH(it,c) for (typeof((c).begin()) it = (c).begin(); it != (c).end(); ++it)


#define MANIK_BANIK    0
#define MANIK_SEKAC    1
#define MANIK_MLATIC   2
#define MANIK_STRAZNIK 3
#define MANIK_SKAUT    4
#define MANIK_STRELEC  5
#define MANIK_LOVEC    6
#define MANIK_KUCHAR   7
#define MANIK_KOVAC    8

#define MANIK_POCET_TYPOV 9


#define MAPA_NEVIEM   0
#define MAPA_OKRAJ    1
#define MAPA_ZELEZO   2
#define MAPA_ZLATO    3
#define MAPA_SUTER    4
#define MAPA_VOLNO    5
#define MAPA_PASCA    6
#define MAPA_START    7

#define priechodne(p) ((p) >= MAPA_VOLNO)


#define PRIKAZ_CHOD 1
#define PRIKAZ_UTOC 2
#define PRIKAZ_KUJ 3
#define PRIKAZ_DAJ_ZLATO 4
#define PRIKAZ_DAJ_ZELEZO 5
#define PRIKAZ_DAJ_SPENAT 6

#define PRIKAZ_POCET_TYPOV 6


struct Bod {
  int x, y;
  Bod() : x(0), y(0) {}
  Bod(int _x, int _y) : x(_x), y(_y) {}
  bool operator==(const Bod& b) const { return x == b.x && y == b.y; }
  bool operator!=(const Bod& b) const { return !(x == b.x && y == b.y); }
  bool operator<(const Bod& b) const { return y < b.y || (y == b.y && x < b.x); }
};


struct Prikaz {
  int kto;   // id manika co to ma vykonat
  int typPrikazu;
  Bod ciel;
  int parameter;   // pre DAJ_* je to mnozstvo, pre KUJ je to typ co vyrabam
  Prikaz() {}
  Prikaz(int _kto, int _typ, Bod _ciel, int _param = 0)
      : kto(_kto), typPrikazu(_typ), ciel(_ciel), parameter(_param) {
  }
  Prikaz(int _kto, int _typ, int _cx, int _cy, int _param = 0)
      : kto(_kto), typPrikazu(_typ), ciel(Bod(_cx, _cy)), parameter(_param) {
  }
};


typedef std::vector<Prikaz> Odpoved;


struct Hrac {
  int skore;
  std::vector<int> mapovanie;   // klienti nevidia
};


struct Manik {
  int id;
  int x;
  int y;
  int ktorehoHraca;   // vy ste 0
  int typ;
  int zlato;
  int zelezo;
  int spenat;
  int kovacEnergia;
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
  std::vector<Manik> manici;   // klienti vidia ciastocne
  Teren teren;   // klienti nevidia, maju iba viditelnyTeren
  int dalsiId;
  int cas;
};


struct Mapa {
  int pocetHracov;
  int w;
  int h;
  Teren pribliznyTeren;   // zlato a zelezo nemaju presne miesta
};


#define FORMAT_VERSION 1

#endif

#ifdef reflection
// tieto udaje pouziva marshal.cpp aby vedel ako tie struktury ukladat a nacitavat

reflection(Bod);
  member(x);
  member(y);
end();

reflection(Prikaz);
  member(kto);
  member(typPrikazu);
  member(ciel);
  member(parameter);
end();

reflection(Hrac);
  member(skore);
  member(mapovanie);
end();

reflection(Manik);
  member(id);
  member(x);
  member(y);
  member(ktorehoHraca);
  member(typ);
  member(zlato);
  member(zelezo);
  member(spenat);
  member(kovacEnergia);
end();

reflection(Teren);
  member(data);
end();

reflection(Stav);
  member(hraci);
  member(manici);
  // teren neserializujeme
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
