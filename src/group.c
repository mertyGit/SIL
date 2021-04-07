/*

   group .c CopyRight 2021 Remco Schellekens, see LICENSE for more details.

   This file contains all functions for grouping of layers 

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sil.h"
#include "log.h"



/*****************************************************************************
  
  create group. Groups are simple dynamicly single linked lists of layers. 

 *****************************************************************************/

SILGROUP *sil_createGroup() {
  SILGROUP *group;

  group=calloc(1,sizeof(SILGROUP));
  if (NULL==group) {
    log_warn("ERR: Can't allocate memory for createGroup");
    sil_setErr(SILERR_NOMEM);
    return NULL;
  }
  group->layer=NULL;
  group->next=NULL;
  sil_setErr(SILERR_ALLOK);
  return group;
}


/*****************************************************************************

   add layer to the group

 *****************************************************************************/

void sil_addLayerGroup(SILGROUP *group, SILLYR *layer) {
  SILGROUP *new,*walk;

  if (NULL==group) {
    log_warn("adding layer to non-initialized group");
    sil_setErr(SILERR_NOMEM);
    return;
  }

  new=sil_createGroup();
  if (NULL==new) return; /* error already logged by createGroup */

  walk=group;
  while(walk->next) walk=walk->next;


  walk->next=new;
  new->layer=layer;

  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************

   remove layer from group. Will not generate error if layer isn't found. 
   You can add the same layer multiple times to the group, but remove 
   will only remove first occurence of layer found 

 *****************************************************************************/

void sil_removeLayerGroup(SILGROUP *group, SILLYR *layer) {
  SILGROUP *found;
  SILGROUP *previous=NULL;

  if (NULL==group) {
    log_warn("removing layer from non-initialized group");
    sil_setErr(SILERR_NOMEM);
    return;
  }

  if (NULL==layer) {
    log_warn("removing non-existing layer from group");
    sil_setErr(SILERR_NOMEM);
    return;
  }
 
  found=group;
  while((found->layer!=layer)&&(found->next)) {
    previous=found;
    found=found->next;
  }
  if (found->layer==layer) {
    if (previous) previous->next=found->next;
    if (found) free(found);
  }

  sil_setErr(SILERR_ALLOK);
}



/*****************************************************************************

  destroy the whole group, freeing memory for each node from bottom till top

 *****************************************************************************/

void sil_destroyGroup(SILGROUP *group) {
  SILGROUP *next;

  if (NULL==group) return;
  do {
    next=group->next;
    free(group);
    group=next;
  } while(next);
  sil_setErr(SILERR_ALLOK);
}

/*****************************************************************************

  show all layers in group

 *****************************************************************************/

void sil_showGroup(SILGROUP *group) {
  SILGROUP *walk;

  if (NULL==group) return;
  walk=group;
  while(walk) {
    if (walk->layer) sil_show(walk->layer);
    walk=walk->next;
  }
}

/*****************************************************************************

  Hide all layers in group

 *****************************************************************************/

void sil_hideGroup(SILGROUP *group) {
  SILGROUP *walk;

  if (NULL==group) return;
  walk=group;
  while(walk) {
    if (walk->layer) sil_hide(walk->layer);
    walk=walk->next;
  }
}
