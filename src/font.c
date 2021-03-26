/*

   font.c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all functions for reading and using fonts in the "angelcode" format
   (check https://www.angelcode.com/products/bmfont/doc/file_format.html ) 
   Note: I don't implemented all options, fonts do have the following constrains:
   - Only a single png file per font, extra will be ignored
   - Its far from unicode ready
   - Textlines within font file may not exceed SILFONT_MAXWIDTH (or are chopped of)

*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "lodepng.h"
#include "sil.h"
#include "log.h"

/*****************************************************************************

  Internal function, Initialize the font context

 *****************************************************************************/


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
  font->outline=0;
}


/*****************************************************************************

  Internal function, get the value back for given (1st occurance) of key in 
  given font. Expected a fully read line with spaces translated to tabs 
  in font->line

 *****************************************************************************/

static char *strValue(SILFONT *font, char *key) {
  char *search=NULL;
  char *foundkey=NULL;
  char *foundval=NULL;
  UINT cnt=0;
  BYTE state=0;

#ifndef SIL_LIVEDANGEROUS
  /* if font isn't initialized properly, get out */
  if (NULL==font) {
    log_warn("searching key/value in a non-initialized font");
    sil_setErr(SILERR_NOTINIT);
    return NULL;
  }
#endif
  
  search=font->line;

  /* scratch will be used as temporary buffer for holding value */
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
            /* thats the key we want  */
            state=2;
          } else {
            /* nope, search further */
            state=0;
          }
        }
        break;
      case 2: /* Have key, so looking for begining of value */
        if (*search!='\t') {
          foundval=search;
          font->scratch[cnt++]=*search;
          state =3;
        } else {
          /* empty value ? end this madness ! */
          state=4;
        }
        break;
      case 3: /* read chars and place it in value until tab (or end) */
        if (*search=='\t') {
          state=4; /* end of value */
        }
        font->scratch[cnt++]=*search;
        break;
    } /* end of switch */
    search++;
  }
  if (font->scratch[0]) {
    sil_setErr(SILERR_ALLOK);
    return font->scratch;
  } else {
    sil_setErr(SILERR_WRONGFORMAT);
    return NULL;
  }
}

/*****************************************************************************

  Internal function : Return value in font->line for given key as int

 *****************************************************************************/
static int intValue(SILFONT *font, char *key) {
  int ret=0;
  char *p=NULL;

  p=strValue(font,key);
  if (p!=NULL) {
    ret=(int)strtol(p,NULL,10);
  }
  sil_setErr(SILERR_ALLOK);
  return ret;
}

		
/*****************************************************************************

  Internal function : Check if section of font file is equal to given section 
  returns 1 if it is , 0 if it isn't

 *****************************************************************************/

static BYTE isSection(SILFONT *font, char *section) {
#ifndef SIL_LIVEDANGEROUS
  /* if font isn't initialized properly, get out */
  if (NULL==font) {
    log_warn("looking for section in a non-initialized font");
    sil_setErr(SILERR_NOTINIT);
    return 0;
  }
#endif
  sil_setErr(SILERR_ALLOK);
  if (0==strncmp(font->line,section,strlen(section))) {
    /* make sure keyword ends here; not triggering 'chars' when looking for 'char' */
    if (font->line[strlen(section)]=='\t') {
      return 1;
    }
  } 
  return 0;
}

/*****************************************************************************

  Load font with given given filename
  return NULL when fontfile can't be found or decoded

 *****************************************************************************/
SILFONT *sil_loadFont(char *name) {
  size_t len=0;
  UINT err=SILERR_ALLOK;
  FILE *fp=NULL;
  signed int c=0;
  int cnt=0;
  int ccnt=0;
  UINT kcnt=0;
  BYTE found,waschar,quote=0;
  char *file=NULL;
  SILFONT *font;


  font=calloc(1,sizeof(SILFONT));
  if (NULL==font) {
    log_info("ERR: Can't allocate memory for addLayer");
    sil_setErr(SILERR_NOMEM);
    return NULL;
  }
  initFont(font); 

  /* Load font files */
  fp=fopen(name,"r");
  if (fp) {

    /* readline line till newline/carriage return and replace all spaces */
    /* (except those between quotes) with tabs                           */
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
            log_warn("File (%s) do have more character definitions then stated at beginning",name);
            sil_setErr(SILERR_WRONGFORMAT);
            free(font);
            return NULL;
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
            log_warn("File (%s) do have more kerning definitions then stated at beginning",name);
            sil_setErr(SILERR_WRONGFORMAT);
            free(font);
            return NULL;
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
              log_err("Can't decode .png file (%s) : %d",file, err);
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
              sil_setErr(SILERR_WRONGFORMAT);
            }
          }
          found=1;
        }

        /* Info section, contains all parameters used to generate font */
        /* ignore it for now                                           */
        if (!(found)&&isSection(font,"info")) {
          font->outline=intValue(font,"outline");
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
    log_err("Can't open given file (%s)",name);
    err=SILERR_CANTOPENFILE;
  }
  if (SILERR_ALLOK!=err) {
    sil_setErr(err);
    /* throw away font if there was an error during decoding */
    free(font);
  }
  return font;
}

/*****************************************************************************

  get character definition of given id (charcode) within font. 
  Will return NULL if not found. 
  character definition is used to locate the character within the image file
  and extra instruction how to draw it, like how many pixels to move the cursor
  to the right and vertical offset

 *****************************************************************************/

SILFCHAR *sil_getCharFont(SILFONT *font, char c) {
#ifndef SIL_LIVEDANGEROUS
  /* if font isn't initialized properly, get out */
  if ((NULL==font)||(NULL==font->image)) {
    log_warn("retrieving charecter information from a non-initialized font");
    sil_setErr(SILERR_NOTINIT);
    return 0;
  }
#endif
  for (int i=0;i<(font->chars);i++) {
    if (font->cdefs[i].id==c) return &font->cdefs[i];
  }
  return NULL;
}

UINT sil_getHeightFont(SILFONT *font) {
#ifndef SIL_LIVEDANGEROUS
  /* if font isn't initialized properly, get out */
  if ((NULL==font)||(NULL==font->image)) {
    log_warn("retrieving charecter information from a non-initialized font");
    sil_setErr(SILERR_NOTINIT);
    return 0;
  }
#endif

  return font->lineHeight+font->outline;
}



/*****************************************************************************

  Find any kerning information in font related to combination of two chars.
  Kerning is used to move characters closer to eachother if the outside boxes
  can overlap without problems, for example, "W" and "A" in most fonts
  Kerning can be disabled by using the SILTXT_NOKERNING flag when drawing text

 *****************************************************************************/
int sil_getKerningFont(SILFONT *font, char first, char second) {
#ifndef SIL_LIVEDANGEROUS
  /* if font isn't initialized properly, get out */
  if ((NULL==font)||(NULL==font->image)) {
    log_warn("retrieving kerning information from a non-initialized font");
    sil_setErr(SILERR_NOTINIT);
    return 0;
  }
#endif
  if ((0==first)||(0==second)) return 0;  /* kerning only usable between *two* existing chars... */
  for (int i=0;i<(font->kernings);i++) {
    if (((char)(font->kdefs[i].first)==first)&&((char)(font->kdefs[i].second)==second)) {
      return font->kdefs[i].amount;
    }
  }
  sil_setErr(SILERR_ALLOK);
  return 0;
}

UINT sil_getOutlineFont(SILFONT *font) {
  return font->outline;
}



/*****************************************************************************

  Get pixel values from fontimage from given x,y position (0,0 is topleft of image file)

 *****************************************************************************/
void sil_getPixelFont(SILFONT *font,UINT x,UINT y, BYTE *red, BYTE *green, BYTE *blue, BYTE *alpha) {
  UINT pos=0;
#ifndef SIL_LIVEDANGEROUS
  /* if font isn't initialized properly, get out */
  if ((NULL==font)||(NULL==font->image)) {
    log_warn("retrieving pixels from a non-initialized font");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif

  if ((x<font->width)&&(y<font->height)) { 
    pos=4*(x+y*(font->width));
    *red  =font->image[pos  ];
    *green=font->image[pos+1];
    *blue =font->image[pos+2];
    *alpha=font->image[pos+3];
  }
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************

  Set alpha factor of font (will be used against alpha values when drawing)

 *****************************************************************************/
void sil_setAlphaFont(SILFONT *font, float alpha) {
#ifndef SIL_LIVEDANGEROUS
  /* if font isn't initialized properly, get out */
  if ((NULL==font)||(NULL==font->image)) {
    log_warn("setting alpha in a non-initialized font");
    sil_setErr(SILERR_NOTINIT);
    return;
  }
#endif
  if (alpha>1) alpha=1;
  if (alpha<=0) alpha=0;
  font->alpha=alpha;
  sil_setErr(SILERR_ALLOK);
}


/*****************************************************************************

   purge font out of memory

 *****************************************************************************/
void sil_destroyFont(SILFONT *font) {
  if (font) {
    if (font->image) free(font->image);
    if (font->cdefs) free(font->cdefs);
    if (font->kdefs) free(font->kdefs);
    free(font);
  }
}
