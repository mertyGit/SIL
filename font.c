#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "lodepng.h"
#include "sil.h"

static void initFont(SILFONT *font) {
  font->image=NULL;
  font->width=0;
  font->height=0;
  font->lineHeight=0;
  font->base=0;
  font->scaleW=0;
  font->scaleH=0;
  font->pages=0;
  font->packed=0;
  font->alphaChnl=0;
  font->redChnl=0;
  font->greenChnl=0;
  font->blueChnl=0;
  font->chars=0;
  font->kernings=0;
  memset(&font->line,0,SILFONT_MAXWIDTH+1);
  memset(&font->scratch,0,SILFONT_MAXWIDTH+1);
  font->cdefs=NULL;
  font->kdefs=NULL;
  font->alpha=1;
  font->mspace=0;
}


/* Search for key in string, return value ( "key=value" ) */
static char *strValue(SILFONT *font, char *key) {
  char *search=NULL;
  char *foundkey=NULL;
  char *foundval=NULL;
  UINT cnt=0;
  BYTE state=0;

  search=font->line;
  memset(font->scratch,0,SILFONT_MAXWIDTH+1);
  while(state<4&&*search) {
    switch (state) {
      case 0: /* looking for tab delimiter */
        if (*search=='\t') {
          foundkey=search+1;
          state=1;
        }
        break;
      case 1: /* looking for end of key, ending with '=' */
        if (*search=='=') {
          if (0==strncmp(foundkey,key,strlen(key))) {
            /* thats the key we want */
            state=2;
          } else {
            /* search for next key */
            state=0;
          }
        }
        break;
      case 2: /* looking for begining of value */
        if (*search!='\t') {
          foundval=search;
          font->scratch[cnt++]=*search;
          state =3;
        } else {
          /* empty value ? end this madness ! */
          state=4;
        }
        break;
      case 3: /* place end of string marker at end of value inside line */
        if (*search=='\t') {
          state=4; /* end of value */
        }
        font->scratch[cnt++]=*search;
        break;
    } /* end of switch */
    search++;
  }
  if (font->scratch[0]) {
    return font->scratch;
  } else {
    return NULL;
  }
}

/* Return value part as int */
static int intValue(SILFONT *font, char *key) {
  int ret=0;
  char *p=NULL;

  p=strValue(font,key);
  if (p!=NULL) {
    ret=(int)strtol(p,NULL,10);
  }
  return ret;
}

		

static BYTE isSection(SILFONT *font, char *section) {
  if (0==strncmp(font->line,section,strlen(section))) {
    /* make sure keyword ends here; not triggering 'chars' when looking for 'char' */
    if (font->line[strlen(section)]=='\t') {
      return 1;
    }
  } 
  return 0;
}

UINT sil_loadFont(SILFONT *font, char *name) {
  size_t len=0;
  UINT err=0;
  FILE *fp=NULL;
  signed int c=0;
  int cnt=0;
  int ccnt=0;
  UINT kcnt=0;
  BYTE found,waschar,quote=0;
  char *file=NULL;

  initFont(font); 


  /* Load font files */
  fp=fopen(name,"r");
  if (fp) {
    do {
      c=fgetc(fp);
      if (c=='"') {
        quote=(0==quote)?1:0; 
      } else {
        if (cnt<SILFONT_MAXWIDTH) {
          if ((c==' ')&&(cnt)) {
            /* ignore leading & double spaces, replace others    */
            /* others with tab for easy search and remove quotes */
            if (!(quote)) {
              if (waschar) {
                font->line[cnt++]='\t';
                waschar=0;
              }
            } else { /* leave space when inside quotes */
              font->line[cnt++]=c;
            }
          } else {   /* not space or newline, so keep it */ 
            if (!isspace(c)) {
              font->line[cnt++]=c;
              waschar=1;
            }
          }
        }
      }

      if ((c=='\r' || c=='\n')&&(cnt)) {
        
        /* found full line, now decode it, based on first keyword */
        found=0;

        /* char definition */
        if (isSection(font,"char")) {
          if (ccnt<font->chars) {
            font->cdefs[ccnt].id=intValue(font,"id");
            font->cdefs[ccnt].x=intValue(font,"x");
            font->cdefs[ccnt].y=intValue(font,"y");
            font->cdefs[ccnt].width=intValue(font,"width");
            font->cdefs[ccnt].height=intValue(font,"height");
            font->cdefs[ccnt].xoffset=intValue(font,"xoffset");
            font->cdefs[ccnt].yoffset=intValue(font,"yoffset");
            font->cdefs[ccnt].xadvance=intValue(font,"xadvance");
            font->cdefs[ccnt].page=intValue(font,"page");
            font->cdefs[ccnt].chnl=intValue(font,"chnl");
            /* in the end, mspace will be the widest amount, used for fixedsize */
            if (font->cdefs[ccnt].xadvance>font->mspace) font->mspace=font->cdefs[ccnt].xadvance;
            ccnt++;
          } else {
            /* more char definitions then given in count on forehand */
            return SILERR_WRONGFORMAT;
          }
        }

        /* Kerning definition */
        if (isSection(font,"kerning")) {
          if (kcnt<font->kernings) {
            font->kdefs[kcnt].first=intValue(font,"first");
            font->kdefs[kcnt].second=intValue(font,"second");
            font->kdefs[kcnt].amount=intValue(font,"amount");
            kcnt++;
          } else {
            /* more kerning definitions then given in count on forehand */
            return SILERR_WRONGFORMAT;
          }
        }

        /* Common section, lots of parameters about the chars */
        if (isSection(font,"common")) {
          font->lineHeight=intValue(font,"lineHeight");
          font->base=intValue(font,"base");
          font->scaleW=intValue(font,"scaleW");
          font->scaleH=intValue(font,"scaleH");
          font->pages=intValue(font,"pages");
          font->packed=intValue(font,"packed");
          font->alphaChnl=intValue(font,"alphaChnl");
          font->redChnl=intValue(font,"redChnl");
          font->greenChnl=intValue(font,"greenChnl");
          font->blueChnl=intValue(font,"blueChnl");
          found=1;
        }

        /* Page section, containing filename of image to load */
        /* only one page / image allowed in this version !    */
        if (!(found)&&isSection(font,"page")) {
          /* find filename and load it */
          err=SILERR_NOFILEFOUND;
          file=strValue(font,"file");
          if (file ) {
            err=lodepng_decode32_file(&font->image,&font->width,&font->height,file);
            if (err) {
              switch (err) {
                case 28:
                case 29:
                  /* wrong file presented as .png */
                  err=SILERR_WRONGFORMAT;
                  break;
                case 78:
                  /* common error, wrong filename */
                  err=SILERR_CANTOPENFILE;
                  break;
                default:
                  /* something wrong with decoding png */
                  err=SILERR_CANTDECODEPNG;
                  break;
              }
            }
          }
          found=1;
        }

        /* Info section, contains all parameters used to generate font */
        /* ignore it for now                                           */
        if (!(found)&&isSection(font,"info")) {
          /* placeholder for future use      */
          found=1;
        }

        /* Chars section, only to get amount of char definitions in file*/
        if (!(found)&&isSection(font,"chars")) {
          font->chars=intValue(font,"count");
          if (font->chars) {
            font->cdefs=calloc(sizeof(SILFCHAR),font->chars);
            if (NULL==font->cdefs) {
              err=SILERR_NOMEM;
            }
          } else {
            /* zero chars ? que ? */
            err=SILERR_NOCHARS;
          }
          found=1;
        }

        /* Kerning section, only to get amount of kerning definitions in file*/
        if (!(found)&&isSection(font,"kernings")) {
          font->kernings=intValue(font,"count");
          if (font->kernings) {
            font->kdefs=calloc(sizeof(SILFKERNING),font->kernings);
            if (NULL==font->kdefs) {
              err=SILERR_NOMEM;
            }
          } 
          found=1;
        }

        memset(font->line,0,SILFONT_MAXWIDTH+1);
        cnt=0;
      }
    } while(c!=EOF && 0==err) ;
    fclose(fp);
  } else {
    err=SILERR_CANTOPENFILE;
  }
  return err;
}

SILFCHAR *sil_getCharFont(SILFONT *font, char c) {
  for (int i=0;i<(font->chars);i++) {
    if (font->cdefs[i].id==c) return &font->cdefs[i];
  }
  return NULL;
}

int sil_getKerningFont(SILFONT *font, char first, char second) {
  if ((0==first)||(0==second)) return 0;  /* kerning only usable between *two* existing chars... */
  for (int i=0;i<(font->kernings);i++) {
    if (((char)(font->kdefs[i].first)==first)&&((char)(font->kdefs[i].second)==second)) {
      return font->kdefs[i].amount;
    }
  }
  return 0;
}

void sil_getPixelFont(SILFONT *font,UINT x,UINT y, BYTE *red, BYTE *green, BYTE *blue, BYTE *alpha) {
  UINT pos=0;

  if ((x<font->width)&&(y<font->height)) { 
    pos=4*(x+y*(font->width));
    *red  =font->image[pos  ];
    *green=font->image[pos+1];
    *blue =font->image[pos+2];
    *alpha=font->image[pos+3];
  }
}

void sil_setAlphaFont(SILFONT *font, float alpha) {
  if (alpha>1) alpha=1;
  if (alpha<=0) alpha=0;
  font->alpha=alpha;
}


/* purge font out of memory, clear contents */
void sil_destroyFont(SILFONT *font) {
  if (font->image) free(font->image);
  if (font->cdefs) free(font->cdefs);
  if (font->kdefs) free(font->kdefs);
  initFont(font);
}
