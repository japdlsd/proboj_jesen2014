
#ifndef UPDATE_H
#define UPDATE_H

#include <ostream>
#include <map>

#include "common.h"

extern const int DX[4];
extern const int DY[4];

extern const int kUtok[MANIK_POCET_TYPOV][MANIK_POCET_TYPOV];
extern const int kTazba[MANIK_POCET_TYPOV];
extern const int kRozhlad[MANIK_POCET_TYPOV];
extern const int kNormalnyKrok[MANIK_POCET_TYPOV];
extern const int kSpenatovyKrok[MANIK_POCET_TYPOV];
extern const int kCenaZlato[MANIK_POCET_TYPOV];
extern const int kCenaZelezo[MANIK_POCET_TYPOV];
extern const int kCenaEnergia[MANIK_POCET_TYPOV];

extern const int kUrodnostNizka;
extern const int kUrodnostVysoka;

extern const int kSkoreFrag;
extern const int kSkoreFragKovac;

extern const int kPopulacnyLimit;
extern const int kZiskaneZlato;
extern const int kZiskaneZelezo;
extern const int kRychlostKuchtenia;
extern const int kSkautHladuje;
extern const int kZaciatocneZlato;
extern const int kZaciatocneZelezo;

extern const int kMaximalnaDlzkaHry;


void zapniObservation(std::ostream* observation);

Stav zaciatokHry(const Mapa& mapa);
void prehladajBfs(const Teren& teren, Bod start, Teren& vzdialenost);
void prehladajLokalneBfs(const Teren& teren, Bod start, int rozsahLimit, std::map<Bod,int>& vzdialenost);
void odsimulujKolo(const Mapa& mapa, Stav& stav, const std::vector<Odpoved>& akcie);
void zistiCoVidi(const Stav& stav, int hrac, Teren& viditelne);
void zamaskujStav(const Mapa& mapa, const Stav& stav, int hrac, const Teren& viditelne, Stav& novy);
void odmaskujOdpoved(const Mapa& mapa, const Stav& stav, int hrac, Odpoved& odpoved);
void zakodujViditelnyTeren(const Teren &vstup, vector<int> &vystup);
void dekodujViditelnyTeren(const vector<int> &vstup, Teren &vystup);
std::vector<int> ktoriZiju(const Mapa& mapa, const Stav& stav);
bool hraSkoncila(const Mapa& mapa, const Stav& stav);
int zistiSkore(const Stav& stav, int hrac);

#endif
