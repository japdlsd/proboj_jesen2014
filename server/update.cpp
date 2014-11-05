
#include <ostream>
#include <vector>
#include <queue>
#include <set>
#include <algorithm>
#include <cmath>
using namespace std;

#include "common.h"
#include "update.h"

// clockwise zhora
const int DX[] = { 0, 1, 0, -1 };
const int DY[] = { -1, 0, 1, 0 };

const int kMaximalnaDlzkaHry = 2000;

const int kPociatocnaSilaBomb = 2; // je to polomer (ak si vzdialeny r policok, tak si v bezpeci)
const int kPociatocnyMaxPocetBomb = 1;

const int kBonusSanca[BONUS_POCET_TYPOV] = 
    {30, 30, 20, 10, 10}; // (percenta) urcuje aky typ bonusu padne, ak padne

const int kBonusZakladnaSanca = 50; // (percenta) sanca, ze bude nejaky bonus

const int kBombaTimer = 10;

const int kStitTrvanie = 20; // pocet kol

const int kOhnostrojKto = -1;
const int kOhnostrojPocet = 6;
const int kOhnostrojStart = 15;
const int kOhnostrojSila = 3;
const int kOhnostrojPocetPokusov = 10;

const int kVianocePocet = 5;
const int kVianocePocetPokusov = 10;


const int kBodyZaHlinu = 3;
const int kBodyZaZabitie = 50;
const int kBodyZaSamovrazdu = -40;
const int kBodyZaPrezitie = 400;

static ostream* g_observation;
void zapniObservation(ostream* observation) { g_observation = observation; }

//@TODO observer

#define OBSERVE(s,...) do {                                                    \
    if (!g_observation) break;                                                 \
    *g_observation << (s);                                                     \
    int __m[] = { __VA_ARGS__ };                                               \
    for (unsigned __i = 0; __i < sizeof(__m)/sizeof(*__m); __i++)              \
      *g_observation << " " << __m[__i];                                       \
    *g_observation << endl;                                                    \
  } while(0)


inline static Hrac vytvorHraca(const Mapa &mapa, const Bod &poloha, const int i){
  Hrac h = Hrac();
  h.x = poloha.x;
  h.y = poloha.y;
  h.skore = 0;
  h.jeZivy = true;
  h.pocetBomb = 0;
  h.maxPocetBomb = kPociatocnyMaxPocetBomb;
  h.silaBomb = kPociatocnaSilaBomb;
  h.maStit = 0;

  // zoberieme nahodnu permutaciu, kde mapovanie[i] == 0
  h.mapovanie.resize(mapa.pocetHracov);
  for (int j = 0; j < mapa.pocetHracov; j++) h.mapovanie[j] = j;
  random_shuffle(h.mapovanie.begin() + 1, h.mapovanie.end());
  swap(h.mapovanie[0], h.mapovanie[i]);

  return h;
}

Stav zaciatokHry(const Mapa& mapa) {
  Stav stav = Stav();   // tento zapis vsetko inicializuje na nuly

  vector<Bod> starty;
  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if (mapa.uvodnyTeren.get(x, y) == MAPA_START) {
      starty.push_back(Bod(x, y));
    }
  }
  random_shuffle(starty.begin(), starty.end());
  
  stav.teren = mapa.uvodnyTeren;

  for (int y = 0; y < mapa.h; y++) for (int x = 0; x < mapa.w; x++) {
    if(mapa.uvodnyTeren.get(x, y) == MAPA_START) {
      stav.teren.set(x, y, MAPA_VOLNO);
    }
  }

  stav.hraci.clear();
  for (int i = 0; i < mapa.pocetHracov; i++) {
    stav.hraci.push_back(vytvorHraca(mapa, starty[i], i));
  }

  return stav;
}

void prehladajBfs(const Teren& teren, const Bod& start, Teren& vzdialenost) {
  int inf = teren.w() * teren.h() * 2;
  vzdialenost.vyprazdni(teren.w(), teren.h(), inf);
  queue<Bod> Q;
  vzdialenost.set(start, 0);
  Q.push(start);
  while (!Q.empty()) {
    Bod p = Q.front(); Q.pop();
    for (int d = 0; d < 4; d++) {
      Bod n(p.x + DX[d], p.y + DY[d]);
      if (teren.get(n) == MAPA_OKRAJ) continue;
      if (vzdialenost.get(n) != inf) continue;
      vzdialenost.set(n, vzdialenost.get(p) + 1);
      if (priechodne(teren.get(n))) Q.push(n);
    }
  }
}

void prehladajLokalneBfs(const Teren& teren, const Bod& start, int rozsahLimit, map<Bod,int>& vzdialenost) {
  vzdialenost.clear();
  queue<Bod> Q;
  vzdialenost[start] = 0;
  Q.push(start);
  while (!Q.empty()) {
    Bod p = Q.front(); Q.pop();
    if (vzdialenost[p] == rozsahLimit) continue;
    for (int d = 0; d < 4; d++) {
      Bod n(p.x + DX[d], p.y + DY[d]);
      if (teren.get(n) == MAPA_OKRAJ) continue;
      if (vzdialenost.count(n)) continue;
      vzdialenost[n] = vzdialenost[p] + 1;
      if (priechodne(teren.get(n))) Q.push(n);
    }
  }
}

inline bool nieJeTuHrac(const Stav& stav, const Bod& p){
  for(int i = 0; i < (int)stav.hraci.size(); i++) 
    if(stav.hraci[i].jeZivy && stav.hraci[i].pozicia() == p)
        return false;
  return true;
}

inline static int ktoJeNaPolicku(const Stav& stav, const Bod& p){
  for(int i = 0; i < mapa.pocetHracov; i++) if(stav.hraci[i].jeZivy){
    if(stav.hraci[i].pozicia() == p) return i;
  }
  return -1;
}

Bomba vytvorBombu(const int id, const int kto, const int sila, const int timer, const Bod& poloha){
  Bomba bomba = Bomba();
  bomba.id = id;
  bomba.kto = kto;
  bomba.sila = sila;
  bomba.timer = timer;
  bomba.x = poloha.x;
  bomba.y = poloha.y;

  return bomba;
}

Bonus vytvorBonus(const int id, const Bod& poloha){
  Bonus bonus = Bonus();
  bonus.id = id;
  bonus.x = poloha.x;
  bonus.y = poloha.y;
  
  int pokus = rand() % 100;
  for(int i = 0; i < BONUS_POCET_TYPOV; i++){
    if(pokus < kBonusSanca[i]){
      bonus.typ = i;
      break;
    }
    else{
      pokus -= kBonusSanca[i];
    }
  }

  return bonus;
}

void aktivujBonus(Stav &stav, Hrac &hrac, const Bonus& bonus, 
    map<Bod, Bonus>& bonusyPodlaPolohy, map<Bod, Bomba>& bombyPodlaPolohy){
  if(bonus.typ == BONUS_SILA){
    hrac.silaBomb += 1;
  }
  else if(bonus.typ == BONUS_STIT){
    hrac.maStit = kStitTrvanie + 1; // pozor, to sa nestackuje (+1 lebo na konci kola sa to odrata)
  }
  else if(bonus.typ == BONUS_POCET){
    hrac.maxPocetBomb += 1;
  }
  else if(bonus.typ == BONUS_OHNOSTROJ){
    // buahahahaha!!!
    int ostavaPokusov = kOhnostrojPocetPokusov;
    int ostavaBomb = kOhnostrojPocet;

    while(ostavaPokusov-- > 0 && ostavaBomb > 0){
      const int x = rand() % stav.teren.w();
      const int y = rand() % stav.teren.h();

      if(stav.teren.get(x, y) == MAPA_VOLNO){ // ano, moze vzniknut aj u hraca pod nohami
        stav.teren.set(x, y, MAPA_BOMBA);
        bombyPodlaPolohy[Bod(x, y)] = vytvorBombu(stav.dalsiId++, kOhnostrojID, kOhnostrojSila, 
            kOhnostrojStart + (kOhnostrojPocet - ostavaBomb) + 1, Bod(x, y));
        ostavaBomb--;
      }
    }
  }
  else if(bonus.typ == BONUS_VIANOCE){
    // Vianoce nie su rekurzivne, nemoze vygnerovat dalsi bonus Vianoce
    // bonus nemoze vzniknut u hraca pod nohami, robilo by to bordel
    int ostavaPokusov = kVianocePocetPokusov;
    int ostavaBonusov = kVianocePocet;

    while(ostavaPokusov-- > 0 && ostavaBonusov){
      const int x = rand() % stav.teren.w();
      const int y = rand() % stav.teren.h();

      if(stav.teren.get(Bod(x, y)) == MAPA_VOLNO && !jeTuHrac(stav, Bod(x, y))){
        Bonus bonus = vytvorBonus(stav.dalsiId++, Bod(x, y));

        if(bonus.typ == BONUS_VIANOCE) continue;
        bonusyPodlaPolohy[Bod(x, y)] = bonus;
        stav.teren.set(Bod(x, y), MAPA_BONUS);

        ostavaBonusov--;
      }
    }
  }
}

inline static int cisloHracaPodlaPolohy(Stav &stav, const Bod& poloha){
  for(int i = 0; i < (int)stav.hraci.size(); i++){
    if(stav.hraci[i].jeZivy && stav.hraci[i].pozicia() == poloha) return i;
  }
  return -1;
}

inline static bool jeTuHrac(STav& stav, const Bod& p){
  return cisloHracaPodlaPolohy(stav, p) > -1;
}

void odsimulujKolo(const Mapa& mapa, Stav& stav, const vector<Odpoved>& akcie) {
  OBSERVE("odsimulujKolo.zacina", stav.cas, stav.cas + 1);
   
  // vykonaju sa prikazy
  //     poradie tahania je urcene nahodne (priorita, ktora sa meni kazde kolo)
  // buchnu bomby, ktore maju timer 0
  //     zarataju sa body
  // znizia sa timery bomb a bonusov
  
  // predpocitavania
  // ====================
  map<Bod, Bonus> bonusyPodlaPolohy;
  FOREACH(it, stav.bonusy) bonusyPodlaPolohy[it->pozicia()] = *it;
  // od tejto chvili sa stav.bonusy nepouziva

  map<Bod, Bomba> bombyPodlaPolohy;
  FOREACH(it, stav.bomby) bombyPodlaPolohy[it->pozicia()] = *it;
  // od tejto chvili sa stav.bomby nepouziva
  
  // vykonavanie prikazov
  // =====================

  // urci sa priorita
  vector<int> priorita(mapa.pocetHracov);
  for(int i = 0; i < mapa.pocetHracov; i++) priorita[i] = i;
  random_shuffle(priorita.begin(), priorita.end());
  
  for(int _prior = 0; _prior < mapa.pocetHracov; _prior++){
    const int i = priorita[_prior];
   
    // @TODO opytat sa Tomiho, ako to vlastne funguje
    // mrtvi netahaju
    if(! stav.hraci[i].jeZivy){
      continue;
    }
  
    Hrac &hrac = stav.hraci[i];
    const Prikaz prikaz = akcie[i];

    if(prikaz.typPrikazu == PRIKAZ_CHOD){
      // zly smer
      if( !(0 <= prikaz.smer && prikaz.smer <= 3)){
        continue;
      }

      const Bod ciel = hrac.pozicia() + Bod(DX[prikaz.smer], DY[prikaz.smer]);

      if(priechodne(stav.teren.get(ciel)) && !jeTuHrac(stav, ciel)){
        hrac.x = ciel.x;
        hrac.y = ciel.y;

        if(stav.teren.get(ciel) == MAPA_BONUS){
          const Bonus bonus = bonusyPodlaPolohy[ciel];
          // @TODO odstranit bonus  
          stav.teren.set(ciel, MAPA_VOLNO);
          bonusyPodlaPolohy.erase(ciel);
          
          aktivujBonus(stav, hrac, bonus, bonusyPodlaPolohy, bombyPodlaPolohy);
        }
      }
    }
    else if(prikaz.typPrikazu == PRIKAZ_POLOZ_BOMBU){
      // uz tam jedna bobma je
      if(stav.teren.get(hrac.pozicia()) == MAPA_BOMBA){
        continue;
      }
      // uz vycerpal limit
      if(hrac.pocetBomb == hrac.maxPocetBomb){
        continue;
      }
      
      bombyPodlaPolohy[hrac.pozicia()] = vytvorBombu(stav.dalsiId++, i, hrac.silaBomb, kBombaTimer + 1, hrac.pozicia());
      stav.teren.set(hrac.pozicia(), MAPA_BOMBA);
      hrac.pocetBomb++;
    }
  }
  
  // idu vybuchovat bomby
  // =======================

  // ako to cele funguje:
  // najprv s pozrieme, 
  //    ktore policka maju byt v ohni, 
  //    ktore bomby maju vybuchnut, 
  //    ktore policka s hlinou maju zaniknut
  //    ktori hraci maju zomriet (anuluju sa im stity, ak mali)
  // nasledne sa pozrieme, kto za co dostane body
  //    body sa prideluju majitelovi bomby, ktora priamo zasahuje objekt znicenia
  //    ak su to viacere bomby, tak su to viaceri hraci. Za kazde trafenie bombou su samostatne body 
  //        (combo, ak trafis ho dvomi bombami v rovnakom case :D)
  // nasledne zomru hraci
  // nasledne zaniknu policka s hlinou, vygeneruju sa bonusy
  // nasledne sa odprataju vybuchnute bomby, znizia sa patricne countery u hracov

  set<Bod> polohyBombCoVybuchnu;
  set<Bod> polohyHlinCoVybuchnu;
  set<int> cislaHracovCoSuVOhni; // iba ti, co nemaju stit
  set<Bod> polickaVOhni;
  
  // vsetko, co pushujeme do Q, su validne policka
  queue<Bod> Q; // policka v ohni
  FOREACH(it, bombyPodlaPolohy) if(it->second.timer == 0){
    Q.push(it->first);
  }

  while(!Q.empty()){
    const Bod poloha = Q.front(); Q.pop();
    
    if(polickaVOhni.find(poloha) != polickaVOhni.end()) continue;

    const int typ = stav.teren.get(poloha);
    
    if(typ == MAPA_VOLNO || typ == MAPA_BONUS){
      polickaVOhni.insert(poloha);
    }
    else if(typ == MAPA_HLINA){
      polohyHlinCoVybuchnu.insert(poloha);
    }
    else if(typ == MAPA_BOMBA){
      polohyBombCoVybuchnu.insert(poloha);
      polickaVOhni.insert(poloha);
      
      Bomba bomba = bombyPodlaPolohy[poloha];
    
      // bombovy "krizik"
      for(int k = 0; k < 4; k++){
        Bod bod = poloha;
        for(int r = 1; r < bomba.sila; r++){
          bod = bod + Bod(DX[k], DY[k]);

          if(stav.teren.get(bod) == MAPA_KAMEN) break;
          if(stav.teren.get(bod) == MAPA_OKRAJ) break;
          if(stav.teren.get(bod) == MAPA_HLINA){
            // tu sa zarataju body za hlinu
            if(bomba.kto >= 0){
              // je to hracova bobma
              stav.hraci[bomba.kto].skore += kBodyZaHlinu;
            }
            
            Q.push(bod);
            break;
          }
        
          {// body za hracov
            int obet = cisloHracaPodlaPolohy(stav, bod);  
            if(obet >= 0 && stav.hraci[obet].maStit > 0) obet = -1; // ti, co maju stit, su nezranitelni
            if(obet >= 0){
              if(obet != bomba.kto){
                stav.hraci[bomba.kto].skore += kBodyZaZabitie;
              }
              else{
                stav.hraci[bomba.kto].skore += kBodyZaSamovrazdu;
              }
            }
          }

          Q.push(bod);
        }
      }
    }
  }
  
  for(int i = 0; i < mapa.pocetHracov; i++) if(polickaVOhni.find(stav.hraci[i].pozicia()) != polickaVOhni.end()){
    if(! stav.hraci[i].maStit) cislaHracovCoSuVOhni.insert(i);
    else stav.hraci[i].maStit = 0;
  }
  
  // upratovanie
  FOREACH(it, cislaHracovCoSuVOhni){
    stav.hraci[*it].jeZivy = false;
  }

  FOREACH(it, polohyHlinCoVybuchnu){
    stav.teren.set(*it, MAPA_VOLNO);

    if(rand()%100 >= kBonusZakladnaSanca){
      stav.teren.set(*it, MAPA_BONUS);
      bonusyPodlaPolohy[*it] = vytvorBonus(stav.dalsiId++, *it);
    }
  }

  FOREACH(it, polohyBombCoVybuchnu){
    Bomba bomba = bombyPodlaPolohy[*it];
    if(bomba.kto >= 0) stav.hraci[bomba.kto].pocetBomb--;
    bombyPodlaPolohy.erase(*it);
  }
  
  // prehodenie veci naspat do stavu
  stav.bomby.clear();
  FOREACH(it, bombyPodlaPolohy){
    stav.bomby.push_back(it->second);
  }

  stav.bonusy.clear();
  FOREACH(it, bonusyPodlaPolohy){
    stav.bonusy.push_back(it->second);
  }

  // znizenie timerov
  FOREACH(it, stav.hraci) if(it->jeZivy){
    it->maStit = max(0, it->maStit - 1);
  }
  
  FOREACH(it, stav.bomby){
    it->timer--;
  }

  OBSERVE("odsimulujKolo.konci", stav.cas, stav.cas + 1);
  stav.cas++;
}

void zamaskujStav(const Mapa& mapa, const Stav& stav, int hrac, const Teren& viditelne, Stav& novy) {
  const vector<int>& mapovanie = stav.hraci[hrac].mapovanie;
  novy = stav;
  novy.hraci.resize(mapa.pocetHracov);
  for (int i = 0; i < mapa.pocetHracov; i++) {
    novy.hraci[mapovanie[i]] = stav.hraci[i];
  }
  novy.mapovanie.clear();
}

void odmaskujOdpoved(const Mapa& mapa, const Stav& stav, int hrac, Odpoved& odpoved) {
  // this function intentionally left blank
}

vector<int> ktoriZiju(const Mapa& mapa, const Stav& stav) {
  vector<int> zijuci;
  for(int i = 0; i < mapa.pocetHracov; i++) if(stav.hraci[i].jeZivy){
    zijuci.push_back(i);
  }
  return zijuci;
}

bool hraSkoncila(const Mapa& mapa, const Stav& stav) {
  int pocetZivychHracov = 0;

  FOREACH(it, stav.hraci) if(it->jeZivy) pocetZivychHracov++;

  return pocetZivychHracov <= 1 || stav.cas >= kMaximalnaDlzkaHry;
}

int zistiSkore(const Stav& stav, int hrac) {
  return stav.hraci[hrac].skore;
}
