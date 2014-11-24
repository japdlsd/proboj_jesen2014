
#include <cstdio>
#include <cstring>
#include <cmath>
#include <vector>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include "SDL.h"
#include <SDL_image.h>
#include "SDL_ttf.h"
using namespace std;

#include "logic.h"
#include "common.h"
#include "marshal.h"
#include "util.h"

#define DEFAULT_SCALE 20
#define FINAL_DELAY 20


static int scale;
static int formatVersion;
static Mapa mapa;
static vector<Stav> stavy;
static vector<pair<string,vector<int> > > observations;
static vector<string> titles;
static map<int, vector<Bod> > coKedyHori;
static map<int, map<int, Bod> > ktoKedyIde;

static set<int> frameTimes;

static TTF_Font *font;
static int fontWidth, fontHeight;
static SDL_Surface *mapSurface;
static SDL_Surface *imgBomba;
static SDL_Surface *imgKamen;
static SDL_Surface *imgHlina;
static SDL_Surface *imgVybuch;
static SDL_Surface *imgBombaCervena;
static SDL_Surface *imgBombaModra;
static SDL_Surface *imgStit;

const int farbyHracov[] = {
  0xC00000,
  0xFF6F00,
  0xA0A0A0,
  0xFFFFFF,
  0x00FF00,
  0xFFFF00,
};

vector<SDL_Surface*> imgHraci(6);
const char* adresyObrazkovHracov[6] = {
  "/figures/veduci.png",
  "/figures/captain_fox.png",
  "/figures/pirate_guineapig.png",
  "/figures/sergeant_kitty.png",
  "/figures/soldier_mouse.png",
  "/figures/viking_chicken.png"
};

vector<SDL_Surface *> imgBonusy(BONUS_POCET_TYPOV);
const char* adresyObrazkovBonusov[BONUS_POCET_TYPOV] = {
  "/figures/bonus_sila.png",
  "/figures/bonus_pocet.png",
  "/figures/bonus_stit.png",
  "/figures/bonus_ohnostroj.png",
  "/figures/bonus_darcek.png",
  "/figures/bonus_freeze.png"
};

template<class T> void checkStream(T& s, string filename) {
  if (s.fail()) {
    fprintf(stderr, "neviem citat z %s\n", filename.c_str());
    exit(1);
  }
}

SDL_Surface* nacitajObrazok(string adresa, string programovyAdresar){
  SDL_Surface *img = IMG_Load((programovyAdresar + adresa).c_str());
  if (img == NULL) {
    fprintf(stderr, "nemam obrazok %s\n", adresa.c_str());
    exit(1);
  }
  return img;
}

SDL_Surface *spravSurface(int w, int h, bool alfa) {
  SDL_Surface *surface = alfa ?
    SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF) :
    SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0xFF0000, 0x00FF00, 0x0000FF, 0x000000);
  if (surface->pitch != surface->w * 4) {
    fprintf(stderr, "surface ma zly pitch\n");
    exit(1);
  }
  return surface;
}

void nacitajMedia(string programovyAdresar) {
  const char *command = "fc-match monospace -f %{file}";
  FILE *pipe = popen(command, "r");
  if (pipe == NULL) {
    fprintf(stderr, "neviem spustit '%s'\n", command);
    exit(1);
  }
  char fontfile[4096 + 1];
  int len = fread(fontfile, 1, 4096, pipe);
  if (ferror(pipe) || !feof(pipe) || !len) {
    fprintf(stderr, "neviem precitat vystup '%s'\n", command);
    exit(1);
  }
  fontfile[len] = 0;

  font = TTF_OpenFont(fontfile, 17); // nastavenie fontu!
  if (!font) {
    fprintf(stderr, "neviem otvorit %s: %s\n", fontfile, TTF_GetError());
    exit(1);
  }

  fontHeight = TTF_FontLineSkip(font);

  SDL_Surface *space = TTF_RenderUTF8_Shaded(font, " ", SDL_Color(), SDL_Color());
  fontWidth = space->w;
  SDL_FreeSurface(space);

  mapSurface = spravSurface(mapa.w*scale, mapa.h*scale, false);

  imgBomba = nacitajObrazok("/figures/bomb.png", programovyAdresar);
  imgHlina = nacitajObrazok("/figures/dirt.png", programovyAdresar);
  imgKamen = nacitajObrazok("/figures/solid.png", programovyAdresar);
  imgVybuch = nacitajObrazok("/figures/explosion.png", programovyAdresar);
  imgBombaCervena = nacitajObrazok("/figures/bomb_red.png", programovyAdresar);
  imgBombaModra = nacitajObrazok("/figures/bomb_blue.png", programovyAdresar);
  imgStit = nacitajObrazok("/figures/shield.png", programovyAdresar);

  for(int i = 0; i < (int)imgHraci.size(); i++){
    imgHraci[i] = nacitajObrazok(adresyObrazkovHracov[i], programovyAdresar);
  }

  for(int i = 0; i < (int)imgBonusy.size(); i++){
    imgBonusy[i] = nacitajObrazok(adresyObrazkovBonusov[i], programovyAdresar);
  }
}


void nacitajAdresar(string zaznamovyAdresar) {
  const char *scalevar = getenv("SCALE");
  scale = atoi(scalevar ? scalevar : "");
  if (!scale) scale = DEFAULT_SCALE;

  ifstream formatstream((zaznamovyAdresar+"/format").c_str());
  formatstream >> formatVersion;
  checkStream(formatstream, zaznamovyAdresar+"/format");
  formatstream.close();

  ifstream observationstream((zaznamovyAdresar+"/observation").c_str());
  string line;
  while (getline(observationstream, line)) {
    stringstream linestream(line);
    string type;
    linestream >> type;
    vector<int> args;
    int arg;
    while (linestream >> arg) args.push_back(arg);
    observations.push_back(make_pair(type, args));
  }
  observationstream.close();
  
  ifstream mapastream((zaznamovyAdresar+"/map").c_str());
  nacitaj(mapastream, mapa);
  checkStream(mapastream, zaznamovyAdresar+"/map");
  mapastream.close();

  ifstream logstream((zaznamovyAdresar+"/log").c_str());
  stavy.resize(1);
  nacitaj(logstream, stavy[0]);
  checkStream(logstream, zaznamovyAdresar+"/log");
  while (true) {
    vector<Odpoved> odpovede;
    Stav s;
    nacitaj(logstream, odpovede);
    nacitaj(logstream, s);
    if (logstream.fail()) break;
    stavy.push_back(s);
  }
  logstream.close();

  ifstream titlestream((zaznamovyAdresar+"/titles").c_str());
  if (titlestream.fail()) {
    for (int i = 0; i < mapa.pocetHracov; i++) {
      stringstream ss;
      ss << "Hrac " << (i+1);
      titles.push_back(ss.str());
    }
  } else {
    for (int i = 0; i < mapa.pocetHracov; i++) {
      string line;
      getline(titlestream, line);
      getline(titlestream, line);
      getline(titlestream, line);
      titles.push_back(line);
    }
    checkStream(titlestream, zaznamovyAdresar+"/titles");
  }
  titlestream.close();

  int cas = 1;
  FOREACH(it, observations) {
    if(it->first == "odsimulujKolo.zacina") cas = it->second[1];
    if(it->first == "Hori")  coKedyHori[cas].push_back(Bod(it->second[0], it->second[1])); 
    if(it->first == "Idem") ktoKedyIde[cas-1][it->second[0]] = Bod(it->second[1], it->second[2]);
  }
}


void zistiVelkostObrazovky(int *w, int *h) {
  // *w = mapa.w*scale;
  // *h = mapa.h*scale + fontHeight * (mapa.pocetHracov + 2);
  *w = mapa.w * scale + fontWidth * 47;
  *h = mapa.h * scale;
}


static void putpixel(int x, int y, Uint32 c, double dx=0, double dy=0) {
  Uint32* pixels = (Uint32 *)mapSurface->pixels;
  x *= scale; y *= scale;
  dx *= scale; dy *= scale;
  for (int xx = 0; xx < scale; xx++) for (int yy = 0; yy < scale; yy++) {
    pixels[int(y+yy+dy) * mapSurface->w + int(x+xx+dx)] = c;
  }
}


static void putimage(int x, int y, SDL_Surface *image, double dx=0, double dy=0, double alpha=1, SDL_Surface *mapSurface=mapSurface) {
  //bleeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
  SDL_Surface *pomocny = spravSurface(image->w, image->h, true);
  SDL_LockSurface(image);
  SDL_LockSurface(pomocny);
  for (int yy = 0; yy < image->h; yy++) for (int xx = 0; xx < image->w; xx++) {
    int c = *((Uint32*)&((Uint8*)image->pixels)[yy * image->pitch + xx*4]);
    Uint8 r, g, b, a; SDL_GetRGBA(c, image->format, &r, &g, &b, &a);
    a = a * alpha;
    c = SDL_MapRGBA(pomocny->format, r, g, b, a);
    *((Uint32*)&((Uint8*)pomocny->pixels)[yy * pomocny->pitch + xx*4]) = c;
  }
  SDL_UnlockSurface(pomocny);
  SDL_UnlockSurface(image);
  //bleeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee

  dx *= scale; dy *= scale;
  x *= scale; x += dx;
  y *= scale; y += dy;
  SDL_Rect r = { (Sint16)x, (Sint16)y, 0, 0 };
  SDL_BlitSurface(pomocny, NULL, mapSurface, &r);
  SDL_FreeSurface(pomocny);
}


class Printer {
public:
  Printer(SDL_Surface *_screen, int _y, int _x) : screen(_screen), x(mapSurface->w + _x), y(_y * fontHeight) {
  }
  void print(const char *text, int width, bool right = true, Uint32 color = 0xFFFFFF) {
    SDL_Color fg; fg.r = (color>>16)&0xFF; fg.g = (color>>8)&0xFF; fg.b = (Uint8)(color&0xFF);
    SDL_Color bg = { 0, 0, 0 };

    if (x != 0) {
      x++;
      SDL_Rect line; line.x = x; line.y = y; line.w = 1; line.h = fontHeight;
      SDL_FillRect(screen, &line, SDL_MapRGB(screen->format, 255, 255, 255));
      x++;
    }

    SDL_Surface *image = TTF_RenderUTF8_Shaded(font, text, fg, bg);
    if(image != NULL){
      SDL_Rect src; src.x = 0; src.y = 0; src.w = min((int)image->w, width * fontWidth); src.h = image->h;
      SDL_Rect dest; dest.x = x + (right && image->w < width * fontWidth ? width * fontWidth - image->w : 0); dest.y = y;
      SDL_BlitSurface(image, &src, screen, &dest);
      SDL_FreeSurface(image);
    }
    x += width * fontWidth;
  }
private:
  SDL_Surface *screen;
  int x, y;
};


void vykresluj(SDL_Surface *screen, double dnow) {
  if (dnow > stavy.size() + FINAL_DELAY) exit(0);
  int now = min((int)dnow, (int)stavy.size() - 1);

  const Stav& stav = stavy[now];

  SDL_LockSurface(mapSurface);
  for (int y = 0; y < mapa.h; y++) {
    for (int x = 0; x < mapa.w; x++) {
      int tuto = stav.teren.get(x, y);
      switch (tuto) {
        case MAPA_HLINA:  putpixel(x, y, 0xFF8800); break;
        case MAPA_KAMEN:  putpixel(x, y, 0x000000); break;
        case MAPA_BONUS:  putpixel(x, y, 0x0000FF); break;
        default:
          putpixel(x, y, 0xC0C0C0);
          break;
      }
    }
  }

  SDL_UnlockSurface(mapSurface);
  
  FOREACH(it, coKedyHori[now]){
    putimage(it->x, it->y, imgVybuch, 0, 0, min(1.0, dnow - now));
  }

  for(int y = 0; y < mapa.h; y++){
    for(int x = 0; x < mapa.w; x++){
      int tuto = stav.teren.get(x,y);
      switch(tuto){
        case MAPA_HLINA: putimage(x, y, imgHlina); break;
        case MAPA_KAMEN: putimage(x, y, imgKamen); break;
      }
    }
  }

  FOREACH(bonus, stav.bonusy){
    putimage(bonus->x, bonus->y, imgBonusy[bonus->typ]);
  }

  FOREACH(it, stav.bomby){
    putimage(it->x, it->y, imgBomba);
    putimage(it->x, it->y, imgBombaCervena, 0, 0, pow(2.71, -it->timer / 2.00));
    if (it->isFrozen) putimage(it->x, it->y, imgBombaModra, 0, 0,   1 - 1/ (pow(2, it->timer/4.00))   );
  }
  
  for(int i = 0; i < mapa.pocetHracov; i++) if(stav.hraci[i].jeZivy){
    const Hrac &hrac = stav.hraci[i];
    if(i < (int)imgHraci.size()){
      putimage(hrac.x, hrac.y, imgHraci[i], (dnow - now)*(ktoKedyIde[now][i].x), (dnow-now)*ktoKedyIde[now][i].y);
    }
    else{
      putpixel(hrac.x, hrac.y, farbyHracov[i],(dnow - now)*(ktoKedyIde[now][i].x), (dnow-now)*ktoKedyIde[now][i].y);
    }
    if(hrac.maStit > 0){
      putimage(hrac.x, hrac.y, imgStit, (dnow - now)*(ktoKedyIde[now][i].x), (dnow-now)*ktoKedyIde[now][i].y, 0.4);

    }
  }

  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
  SDL_BlitSurface(mapSurface, NULL, screen, NULL);

  Printer header(screen, 0, mapa.w);
  header.print("Hráč", 20, false); header.print("Skóre", 5);
  header.print("Počet", 6);
  header.print("Sila", 5);
  header.print("Štít", 5);

  SDL_Rect hline; hline.x = mapSurface->w; hline.y = fontHeight - 1; hline.w = screen->w; hline.h = 1;
  SDL_FillRect(screen, &hline, SDL_MapRGB(screen->format, 255, 255, 255));

  for (int i = 0; i < mapa.pocetHracov; i++) {
    Printer p(screen, i + 1, mapa.w);
    const Hrac& hrac = stav.hraci[i];
    putimage(mapa.h, 1 + i, imgHraci[i], 0, 0, 1, screen); // @TODO;
    p.print(titles[i].c_str(), 20, false, farbyHracov[i]);
    p.print(itos(hrac.skore).c_str(), 5);
    p.print(itos(hrac.maxPocetBomb).c_str(), 6);
    p.print(itos(hrac.silaBomb).c_str(), 5);
    p.print(( (hrac.maStit)?itos(hrac.maStit).c_str():"" ), 5);
  }

  int realtimenow = SDL_GetTicks();
  frameTimes.insert(realtimenow);
  while (*frameTimes.begin() < realtimenow - 5000) frameTimes.erase(frameTimes.begin());

  char buf[1000];
  sprintf(buf, "čas %d", now);
  if (realtimenow > 5000) sprintf(buf+strlen(buf), ", fps %.1f", frameTimes.size()/5.0);
  Printer(screen, mapa.pocetHracov + 1, mapa.w).print(buf, 30, false);
}
