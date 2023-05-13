/**********************************************************************/
/*           automatic test pattern generation                        */
/*           ATPG class header file                                   */
/*                                                                    */
/*           Author: Bing-Chen (Benson) Wu                            */
/*           last update : 01/21/2018                                 */
/**********************************************************************/

#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <forward_list>
#include <array>
#include <memory>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#define HASHSIZE 3911

/* gate type of node,  used in class NODE */
#define NOT       1
#define NAND      2
#define AND       3
#define INPUT     4  // primary input
#define NOR       5
#define OR        6
#define OUTPUT    8  // primary output
#define XOR      11
#define BUF      17  // can be a buffer or a fanout stem 
#define EQV          0    /* XNOR gate */
#define SCVCC    20


/* miscellaneous substitutions */
#define MAYBE          2
#define TRUE           1
#define FALSE          0
#define REDUNDANT      3
#define STUCK0         0
#define STUCK1         1
#define STR            0
#define STF            1
#define ALL_ONE        0xffffffff // for parallel fault sim; 2 ones represent a logic one
#define ALL_ZERO       0x00000000 // for parallel fault sim; 2 zeros represent a logic zero

/* GI=GateInput   GO=GateOutput*/
#define GI 0
#define GO 1

/* 4-valued logic */
#define U  2      // unknown
#define D  3
#define D_bar  4

using namespace std;

/* this is an ATPG solver */
class ATPG {
 public:

  ATPG();

  /* defined in main.cpp */
  void set_fsim_only(const bool &);
  void set_tdfsim_only(const bool &);
  void read_vectors(const string &);
  void set_total_attempt_num(const int &);
  void set_backtrack_limit(const int &);

  /* defined in input.cpp */
  void input(const string &);
  void timer(FILE *, const string &);

  /* defined in level.cpp */
  void level_circuit();
  void rearrange_gate_inputs();

  /* defined in init_flist.cpp */
  void create_dummy_gate();
  void generate_fault_list();
  void compute_fault_coverage();

  /*defined in tdfsim.cpp*/
  void generate_tdfault_list();
  void transition_delay_fault_simulation(int &);
  void tdfault_sim_a_vector(const string &, int &);
  void tdfault_sim_a_vector2(const string &, int &);
  int num_of_tdf_fault{};
  int detected_num{};
  bool get_tdfsim_only() { return tdfsim_only; }

  /* defined in atpg.cpp */
  void test();

 private:

  /* alias declaration */
  class WIRE;
  class NODE;
  class FAULT;
  typedef WIRE *wptr;                 /* using pointer to access/manipulate the instances of WIRE */
  typedef NODE *nptr;                 /* using pointer to access/manipulate the instances of NODE */
  typedef FAULT *fptr;                 /* using pointer to access/manipulate the instances of FAULT */
  typedef unique_ptr<WIRE> wptr_s;    /* using smart pointer to hold/maintain the instances of WIRE */
  typedef unique_ptr<NODE> nptr_s;    /* using smart pointer to hold/maintain the instances of NODE */
  typedef unique_ptr<FAULT> fptr_s;    /* using smart pointer to hold/maintain the instances of FAULT */


  /* fault list */
  forward_list<fptr_s> flist;          /* fault list */
  forward_list<fptr> flist_undetect;   /* undetected fault list */

  /* circuit */
  vector<wptr> sort_wlist;             /* sorted wire list with regard to level */
  vector<wptr> cktin;                  /* input wire list */
  vector<wptr> cktout;                 /* output wire list */

  /* for parsing circuits */
  array<forward_list<wptr_s>, HASHSIZE> hash_wlist;   /* hashed wire list */
  array<forward_list<nptr_s>, HASHSIZE> hash_nlist;   /* hashed node list */

  /* test vector  */
  int in_vector_no;                    /* number of test vectors generated */
  vector<string> vectors;              /* vector set */

  /* declared in tpgmain.cpp */
  int backtrack_limit;
  int total_attempt_num;               /* number of test generation attempted for each fault  */
  bool fsim_only;                      /* flag to indicate fault simulation only */
  bool tdfsim_only;                    /* flag to indicate tdfault simulation only */

  /* used in input.cpp to parse circuit*/
  int debug;                           /* != 0 if debugging;  this is a switch of debug mode */
  string filename;                     /* current input file */
  int lineno;                          /* current line number */
  string targv[100];                   /* tokens on current command line */
  int targc;                           /* number of args on current command line */
  int file_no;                         /* number of current file */
  double StartTime{}, LastTime{};
  int hashcode(const string &);
  wptr wfind(const string &);
  nptr nfind(const string &);
  wptr getwire(const string &);
  nptr getnode(const string &);
  void newgate();
  void set_output();
  void set_input(const bool &);
  void parse_line(const string &);
  void create_structure();
  int FindType(const string &);
  void error(const string &);
  void display_circuit();

  /*  in init_flist.cpp */
  int num_of_gate_fault;    // total number of gate-level uncollapsed faults in the whole circuit

  char itoc(const int &);

  /* declared in sim.cpp */
  void sim();
  void evaluate(nptr);
  int ctoi(const char &);

  /* declared in faultsim.cpp */
  unsigned int Mask[16] = {0x00000003, 0x0000000c, 0x00000030, 0x000000c0,
                           0x00000300, 0x00000c00, 0x00003000, 0x0000c000,
                           0x00030000, 0x000c0000, 0x00300000, 0x00c00000,
                           0x03000000, 0x0c000000, 0x30000000, 0xc0000000,};
  unsigned int Unknown[16] = {0x00000001, 0x00000004, 0x00000010, 0x00000040,
                              0x00000100, 0x00000400, 0x00001000, 0x00004000,
                              0x00010000, 0x00040000, 0x00100000, 0x00400000,
                              0x01000000, 0x04000000, 0x10000000, 0x40000000,};

  // faulty wire linked list.  used to record all wires on fault propagation path 
  // in the event-driven order  
  forward_list<wptr> wlist_faulty;

  void fault_simulate_vectors(int &);
  void fault_sim_a_vector(const string &, int &);
  void fault_sim_evaluate(wptr);
  wptr get_faulty_wire(fptr, int &);
  void inject_fault_value(wptr, const int &, const int &);
  void combine(wptr, unsigned int &);
  unsigned int PINV(const unsigned int &);
  unsigned int PEXOR(const unsigned int &, const unsigned int &);
  unsigned int PEQUIV(const unsigned int &, const unsigned int &);

  /* declared in podem.cpp */
  int no_of_backtracks{};  // current number of backtracks
  bool find_test{};        // true when a test pattern is found
  bool no_test{};          // true when it is proven that no test exists for this fault

  int podem(fptr, int &);
  wptr fault_evaluate(fptr);
  void forward_imply(wptr);
  wptr test_possible(fptr);
  wptr find_pi_assignment(wptr, const int &);
  wptr find_hardest_control(nptr);
  wptr find_easiest_control(nptr);
  nptr find_propagate_gate(const int &);
  bool trace_unknown_path(wptr);
  bool check_test();
  void mark_propagate_tree(nptr);
  void unmark_propagate_tree(nptr);
  int set_uniquely_implied_value(fptr);
  int backward_imply(wptr, const int &);

  /* declared in display.cpp */
  void display_line(fptr);
  void display_io();
  void display_undetect();
  void display_fault(fptr);

  /* declaration of WIRE, NODE, and FAULT classes */
  /* in our model, a wire has inputs (inode) and outputs (onode) */
  class WIRE {
   public:
    WIRE();

    string name;               /* ascii name of wire */
    vector<nptr> inode;        /* nodes driving this wire */
    vector<nptr> onode;        /* nodes driven by this wire */
    int value;                 /* logic value [0|1|2] of the wire (2 = unknown)  
	                             NOTE: we use [0|1|2] in fault-free sim 
								 but we use [00|11|01] in parallel fault sim  */
    int level;                 /* level of the wire */
    int wire_value1;           /* (32 bits) represents fault-free value for this wire. 
                                  the same [00|11|01] replicated by 16 times (for pfedfs) */
    int wire_value2;           /* (32 bits) represents values of this wire 
                                  in the presence of 16 faults. (for pfedfs) */
    int wlist_index;           /* index into the sorted_wlist array */

    //  the following functions control/observe the state of wire
    //  HCY 2020/2/6
    void set_(int type) { flag |= type; }
    void set_scheduled() { flag |= 1; }        // Set scheduled when the input of the gate driving it change.
    void set_all_assigned() { flag |= 2; }     // Set all assigned if both assign 0 and assign 1 already tried.
    void set_input() { flag |= 4; }            // Set input if the wire is PI.
    void set_output() { flag |= 8; }           // Set output if the wire is PO.
    void set_marked() { flag |= 16; }          // Set marked when the wire is already leveled.
    void set_fault_injected() { flag |= 32; }  // Set fault injected if fault inject to the wire.
    void set_faulty() { flag |= 64; }          // Set faulty if the wire is faulty.
    void set_changed() { flag |= 128; }        // Set changed if the logic value on this wire has recently been changed.
    void remove_(int type) { flag &= ~type; }
    void remove_scheduled() { flag &= ~1; }
    void remove_all_assigned() { flag &= ~2; }
    void remove_input() { flag &= ~4; }
    void remove_output() { flag &= ~8; }
    void remove_marked() { flag &= ~16; }
    void remove_fault_injected() { flag &= ~32; }
    void remove_faulty() { flag &= ~64; }
    void remove_changed() { flag &= ~128; }
    bool is_(int type) { return flag & type; }
    bool is_scheduled() { return flag & 1; }
    bool is_all_assigned() { return flag & 2; }
    bool is_input() { return flag & 4; }
    bool is_output() { return flag & 8; }
    bool is_marked() { return flag & 16; }
    bool is_fault_injected() { return flag & 32; }
    bool is_faulty() { return flag & 64; }
    bool is_changed() { return flag & 128; }
    void set_fault_free() { fault_flag &= ALL_ZERO; }
    void inject_fault_at(int bit_position) { fault_flag |= (3 << (bit_position << 1)); }  //  inject a fault at bit position.  Two bits (11) means this position is a fault.  When parallel fault sim, this corresponds to fault position in wire_value2 
    bool has_fault_at(int bit_position) { return fault_flag & (3 << (bit_position << 1)); }
   private:
    int flag = 0;                  /* 32 bit, records state of wire */
    int fault_flag = 0;            /* 32 bit, indicates the fault-injected bit position, for pfedfs */

  };// class WIRE

  //   this is a schematic for fanout
  //   wire - node (GATE) - wire - node (GATE)
  //                             \ node (GATE)    
  //                             \ node (GATE)
  //   
  class NODE {
   public:
    NODE();

    string name;               /* ascii name of node */
    vector<wptr> iwire;        /* wires driving this node */
    vector<wptr> owire;        /* wires driven by this node */
    int type;                  /* node type,  AND OR BUF INPUT OUTPUT ... */
    void set_marked() { marked = true; }          // Set marked if this node is on the path to PO.
    void remove_marked() { marked = false; }
    bool is_marked() { return marked; }
   private:
    bool marked;
  };

  // A fault is defined on a wire.
  // The fault name is associated with a node.
  // If there is no fanout:
  //   wireA \ 
  //         node1  - wireC - node2
  //   wireB / 
  //  then wireC has two faults, associated with "node1 output faults"
  //
  // if there is fanout 
  //   wireA - node1 - wireB - node2 
  //                         \ node3   
  //                         \ node4 
  //      wireB has 8 faults: 
  //       two are associated with "node1 output faults", 
  //       two are associated with "node2 input faults",
  //       two are associated with "node3 input faults",
  //       two are associated with "node4 input faults".
  //
  class FAULT {
   public:
    FAULT();

    nptr node;                 /* gate under test(NIL if PI/PO fault) */
    short io;                  /* 0 = GI; 1 = GO */
    short index;               /* index for GI fault. it represents the  
			                            associated gate input index number for this GI fault */
    short fault_type;          /* s-a-1 or s-a-0 or slow-to-rise or slow-to-fall fault */
    short detect;              /* detection flag */
    short activate{};            /* activation flag */
    bool test_tried;           /* flag to indicate test is being tried */
    int eqv_fault_num;         /* accumulated number of equivalent faults from PI to PO, see initflist.cpp */
    int to_swlist;             /* index to the sort_wlist[] */
    int fault_no;              /* fault index */
    int detected_time{};         /* for N-detect */
  }; // class FAULT
};// class ATPG
