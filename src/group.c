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
    return NULL;
  }
  group->layer=NULL;
  group->next=NULL;
  return group;
}


/*****************************************************************************

   add layer to the group

 *****************************************************************************/

void sil_addLayerGroup(SILGROUP *group, SILLYR *layer) {
  SILGROUP *new,*walk;

  if (NULL==group) {
    log_warn("adding layer to non-initialized group");
    return;
  }

  new=sil_createGroup();
  if (NULL==new) return; /* error already logged by createGroup */

  walk=group;
  while(walk->next) walk=walk->next;


  walk->next=new;
  new->layer=layer;

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
    return;
  }

  if (NULL==layer) {
    log_warn("removing non-existing layer from group");
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

/*****************************************************************************

  Move all layers relative to current postions (so x,y can be negative )

 *****************************************************************************/

void sil_moveGroup(SILGROUP *group,int x, int y) {
  SILGROUP *walk;

  if (NULL==group) return;
  walk=group;
  while(walk) {
    if (walk->layer) sil_moveLayer(walk->layer,x,y);
    walk=walk->next;
  }
}


/*****************************************************************************

  all layers in group switch to next sprite

 *****************************************************************************/

void sil_nextSpriteGroup(SILGROUP *group) {
  SILGROUP *walk;

  if (NULL==group) return;
  walk=group;
  while(walk) {
    if (walk->layer) sil_nextSprite(walk->layer);
    walk=walk->next;
  }
}

/*****************************************************************************

  all layers in group switch to previous sprite

 *****************************************************************************/

void sil_prevSpriteGroup(SILGROUP *group) {
  SILGROUP *walk;

  if (NULL==group) return;
  walk=group;
  while(walk) {
    if (walk->layer) sil_prevSprite(walk->layer);
    walk=walk->next;
  }
}

/*****************************************************************************

  all layers in group switch to given sprite #

 *****************************************************************************/

void sil_setSpriteGroup(SILGROUP *group,UINT num) {
  SILGROUP *walk;

  if (NULL==group) return;
  walk=group;
  while(walk) {
    if (walk->layer) sil_setSprite(walk->layer,num);
    walk=walk->next;
  }
}

/*****************************************************************************

  check if given layer is part of the group; returns amount of occurences 
  in group for this layer

 *****************************************************************************/

UINT sil_checkLayerGroup(SILGROUP *group,SILLYR *layer) {
  SILGROUP *walk;
  UINT cnt=0;

  if (NULL==group) return 0;
  walk=group;
  while(walk) {
    if ((walk->layer) && (walk->layer==layer)) cnt++;
    walk=walk->next;
  }
  return cnt;
}

/*****************************************************************************

  Resets the view of all layers in group

 *****************************************************************************/


void sil_resetViewGroup(SILGROUP *group) {
  SILGROUP *walk;
  UINT cnt=0;

  if (NULL==group) return;
  walk=group;
  while(walk) {
    if (walk->layer) sil_resetView(walk->layer);
    walk=walk->next;
  }
}

/*****************************************************************************

  brings all layers of group to top

 *****************************************************************************/


void sil_topGroup(SILGROUP *group) {
  SILGROUP *walk;
  UINT cnt=0;

  if (NULL==group) return;
  walk=group;
  while(walk) {
    if (walk->layer) sil_toTop(walk->layer);
    walk=walk->next;
  }
}

/*****************************************************************************

  brings all layers of group to bottom

 *****************************************************************************/


void sil_bottomGroup(SILGROUP *group) {
  SILGROUP *walk;
  UINT cnt=0;

  if (NULL==group) return;
  walk=group;
  while(walk) {
    if (walk->layer) sil_toBottom(walk->layer);
    walk=walk->next;
  }
}
