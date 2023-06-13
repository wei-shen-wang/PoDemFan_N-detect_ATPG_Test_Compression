/**********************************************************************/
/*           This is the logic simulator for atpg                     */
/*                                                                    */
/*           Author: Bing-Chen (Benson) Wu                            */
/*           last update : 01/21/2018                                 */
/**********************************************************************/

#include "atpg.h"
#include "logic_tbl.h"

/*
*   sim
*
*   RETURNS
*       no value is returned
*
*   SIDE-EFFECTS
*       changes the logic values inside some wires' data structures
*
*   perform the logic simulation 
*   event-driven technique coupled with levelling mechanism is employed
*   single pattern, no fault injected
*/

void ATPG::sim() {
  int i, j, ncktin, nout, nckt;

  ncktin = cktin.size();
  nckt = sort_wlist.size();
  /* for every input */
  /*TODO*/
  // Schedule every gate connected to a changed input.
  // evaluate() every scheduled gate & propagate any changes.
  // walk through all wires in increasing order.
  // Because the wires are sorted according to their levels,
  // it is correct to evaluate the wires in increasing order.
  /*
   * For event-driven simulation, we set_change() after we inject new values (all ncktin wires)
   * So, if a wire value is_changed(), we should remove this flag and set_schedule() wait for simulation.
   * We can then start to simulate circuit by sort_wlist since it is sorted by level
   * Using evaluate() on connected node and schedule next wire if value changed
   */
  for (i = 0; i < ncktin; i++) {

    /* if a input has changed, schedule the gates connected to it */
    if (sort_wlist[i]->is_changed()) {
      sort_wlist[i]->remove_changed();
      for (j = 0, nout = sort_wlist[i]->onode.size(); j < nout; j++) {
        if (!sort_wlist[i]->onode[j]->owire.empty()) {
          sort_wlist[i]->onode[j]->owire.front()->set_scheduled();
        }
      }
    }
  } // for every input




  for (i = ncktin; i < nckt; i++) {
    if (sort_wlist[i]->is_scheduled()) {
      sort_wlist[i]->remove_scheduled();
      evaluate(sort_wlist[i]->inode.front());
      if (sort_wlist[i]->is_changed()) {
        sort_wlist[i]->remove_changed();
        for (j = 0, nout = sort_wlist[i]->onode.size(); j < nout; j++) {
          if (!sort_wlist[i]->onode[j]->owire.empty()) {
            sort_wlist[i]->onode[j]->owire.front()->set_scheduled();
          }
        }
      }
    }
  }
  /*end  TODO*/



}/* end of sim */

void ATPG::evaluate(nptr n) {
  int old_value, new_value;
  int i, nin;

  old_value = n->owire.front()->value;

  /* decompose a multiple-input gate into multiple levels of two-input gates
   * then look up the truth table of each two-input gate
   */
  nin = n->iwire.size();
  switch (n->type) {
    case AND:
    case BUF:
    case NAND:
      new_value = 1;
      for (i = 0; i < nin; i++) {
        new_value = ANDTABLE[n->iwire[i]->value][new_value];
      }
      if (n->type == NAND) {
        new_value = INV[new_value];
      }
      break;
    case OR:
    case NOR:
      new_value = 0;
      for (i = 0; i < nin; i++) {
        new_value = ORTABLE[n->iwire[i]->value][new_value];
      }
      if (n->type == NOR) {
        new_value = INV[new_value];
      }
      break;
    case NOT:
      new_value = INV[n->iwire.front()->value];
      break;
    case XOR:
      new_value = XORTABLE[n->iwire[0]->value][n->iwire[1]->value];
      break;
    case EQV:
      new_value = INV[(XORTABLE[n->iwire[0]->value][n->iwire[1]->value])];
      break;
  }
  if (old_value != new_value) {
    n->owire.front()->set_changed();
    n->owire.front()->value = new_value;
  }
}/* end of evaluate */

int ATPG::ctoi(const char &c) {
  if (c == '2') return (2);
  if (c == '1') return (1);
  if (c == '0') return (0);
}
