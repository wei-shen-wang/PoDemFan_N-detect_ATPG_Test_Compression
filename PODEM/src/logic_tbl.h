/**********************************************************************/
/*                    five-valued logic table                         */
/**********************************************************************/

#define MULTIVALUE 5
#define U  2
#define D  3
#define B  4  // D_bar

/* five-valued truth table */          // 0   1   U   D   B
int ANDTABLE[MULTIVALUE][MULTIVALUE] = {0, 0, 0, 0, 0,  //0
                                        0, 1, U, D, B,  //1
                                        0, U, U, U, U,  //U
                                        0, D, U, D, 0,  //D
                                        0, B, U, 0, B};//B

int ORTABLE[MULTIVALUE][MULTIVALUE] = {0, 1, U, D, B,
                                       1, 1, 1, 1, 1,
                                       U, 1, U, U, U,
                                       D, 1, U, D, 1,
                                       B, 1, U, 1, B};

int XORTABLE[MULTIVALUE][MULTIVALUE] = {0, 1, U, D, B,
                                        1, 0, U, B, D,
                                        U, U, U, U, U,
                                        D, B, U, 0, 1,
                                        B, D, U, 1, 0};

int EQVTABLE[MULTIVALUE][MULTIVALUE] = {1, 0, U, B, D,
                                        0, 1, U, D, B,
                                        U, U, U, U, U,
                                        B, D, U, 1, 0,
                                        D, B, U, 0, 1};

int INV[MULTIVALUE] = {1, 0, U, B, D};
