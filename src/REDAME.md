
***************************************************

  Utilities for ECE255: VLSI TESTING TECHNIQUES

***************************************************

<<< Sub-directory list: >>>
===========================

1. src: A combinational test generator (taking .sim circuit format)

2. sample_circuits: benchmark circuits in .sim format

3. ltest: A mini testing package from USC. 
          including: * Logic simulator
                     * Combinational test generator (D-algorithm)
                     * Deductive fault simulator
          input format: see examples in ./ltest/circuits.
          detailed info: see README in ./ltest


<<< Basic Data Structures for podem >>>
========================================
1. (NETLIST) The internal data structure of a netlist constructed in podem 
can be described by a set of connected NODEs and WIREs.
The definition of NODE and WIRE is described in header file: atpg.h

2. (IO) Primary inputs (outputs) are treated as pseudo WIREs, and can be
accessed by the global array variable: cktin (cktout), defined in global.h. 
(The fanin (fanout) node of a PI (PO) is a empty vector).

3. (WIRE) In general, a WIRE could have multiple fanin's and multiple fanout's.
But usually in the netlist, a wire only has a single driving (fanin) 
node, though it may have multiple fanout nodes. 

<<< Extracted data structure of WIRE from atpg.h>>>
===================================================
class WIRE;
typedef WIRE*  wptr;
typedef unique_ptr<WIRE>  wptr_s;
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
    void set_(int type) { flag |= type; }     
    void set_scheduled() {flag |= 1;}        // Set scheduled when the input of the gate driving it change.
    void set_all_assigned() {flag |= 2;}     // Set all assigned if both assign 0 and assign 1 already tried.
    void set_input() {flag |= 4;}            // Set input if the wire is PI.
    void set_output() {flag |= 8;}           // Set output if the wire is PO.
    void set_marked() {flag |= 16;}          // Set marked when the wire is already leveled.
    void set_fault_injected() {flag |= 32;}  // Set fault injected if fault inject to the wire.
    void set_faulty() {flag |= 64;}          // Set faulty if the wire is faulty.
    void set_changed() {flag |= 128;}        // Set changed if the logic value on this wire has recently been changed.
    void remove_(int type) {flag &= ~type;}  
    void remove_scheduled() {flag &= ~1;}
    void remove_all_assigned() {flag &= ~2;}
    void remove_input() {flag &= ~4;}
    void remove_output() {flag &= ~8;}
    void remove_marked() {flag &= ~16;}
    void remove_fault_injected() {flag &= ~32;}
    void remove_faulty() {flag &= ~64;}
    void remove_changed() {flag &= ~128;}
    bool is_(int type) {return flag & type;}  
    bool is_scheduled() {return flag & 1;}    
    bool is_all_assigned() {return flag & 2;}
    bool is_input() {return flag & 4;}
    bool is_output() {return flag & 8;}
    bool is_marked() {return flag & 16;}
    bool is_fault_injected() {return flag & 32;}
    bool is_faulty() {return flag & 64;}
    bool is_changed() {return flag & 128;}
    void set_fault_free() {fault_flag &= ALL_ZERO;}
    void inject_fault_at(int bit_position) {fault_flag |= (3 << bit_position);}
    bool has_fault_at(int bit_position) {fault_flag & (3 << bit_position);}
  private:
    int flag = 0;                  /* 32 bit, records state of wire */
    int fault_flag = 0;            /* 32 bit, indicates the fault-injected bit position, for pfedfs */
};// class WIRE
    this is a schematic for fanout
    wire - node (GATE) - wire - node (GATE)
                              \ node (GATE)    
                              \ node (GATE)    


4. (NODE) In general, a NODE could have multiple fanin's and multiple fanout's.
But usually a node in the netlist has only a single fanout WIRE.

<<< Extracted data structure of NODE from atpg.h >>>
====================================================
class NODE {
  public:
    NODE();
    string name;               /* ascii name of node */
    vector<wptr> iwire;        /* wires driving this node */
    vector<wptr> owire;        /* wires driven by this node */
    int type;                  /* node type,  AND OR BUF INPUT OUTPUT ... */
	  void set_marked() {marked = true;}          // Set marked if this node is on the path to PO.
	  void remove_marked() {marked = false;}
	  bool is_marked() {return marked;}
  private:
    bool marked;
};

    A fault is defined on a wire.
    The fault name is associated with a node.
    If there is no fanout:
    wireA \ 
           node1  - wireC - node2
    wireB / 
    then wireC has two faults, associated with "node1 output faults"
   
    if there is fanout 
    wireA - node1 - wireB - node2 
                          \ node3   
                          \ node4 
    wireB has 8 faults: 
        two are associated with "node1 output faults", 
        two are associated with "node2 input faults",
        two are associated with "node3 input faults",
        two are associated with "node4 input faults".
  
5. (GATE TYPE) The supported gate type is defined in atpg.h.

<<< Extracted gate type from atpg.h>>>
=======================================
    #define NOT       1
    #define NAND      2
    #define AND       3
    #define INPUT     4
    #define NOR       5
    #define OR        6
    #define OUTPUT    8
    #define XOR      11
    #define BUF      17

6. (FAULT) Data structure for a stuck-at fault is defined in atpg.h.

<<< Extracted data strcuture of FAULT from atpg.h >>>
=======================================================
class FAULT;
typedef FAULT* fptr;
typedef unique_ptr<FAULT> fptr_s;
class FAULT {
public:
    FAULT();              /* constructor */
    nptr node;            /* gate under test(NIL if PI/PO fault) */
    short io;             /* 0 = GI; 1 = GO */
    short index;          /* index for GI fault */
    short fault_type;     /* s-a-1 or s-a-0 fault */
    short detect;         /* detection flag */
    bool  test_tried;     /* flag to indicate test is being tried */
    int   eqv_fault_num;  /* number of equivalent faults */
    int   to_swlist;      /* index to the sort_wlist[] */
    int   fault_no;       /* fault index */
};

7. (GLOBAL VARIABLES INSIDE THE ATPG CLASS) Several global variables,
such as the number of primary inputs and outputs, are defined in header
file atpg.h.

NOTICE! Alghough these variables are said to be global, they are
only global INSIDE the ATPG class. i.e. they are local variables
of ATPG class.

<<< Extracted global variables from global.h >>>
================================================
vector<wptr> sort_wlist;                         /* sorted wire list with regard to level */
vector<wptr> cktin;                              /* input wire list */
vector<wptr> cktout;                             /* output wire list */
array<forward_list<wptr_s>,HASHSIZE> hash_wlist; /* hashed wire list */
array<forward_list<nptr_s>,HASHSIZE> hash_nlist; /* hashed node list */
int  in_vector_no;                               /* number of test vectors generated */
bool fsim_only;                                  /* flag to indicate fault simulation only */
int  sim_vectors;                                /* number of simulation vectors */
vector<string> vectors;                          /* vector set */


<<< Sample code segments >>>
============================

1. Enumerate all primary inputs
   ***************************************************
   list_all_PI()
   {
      int i = 0;

      for(wptr pi_wire: cktin){
           printf("%d-th PI wire: %s\n", i, pi_wire->name.cstr());
           i++;
      }
   }
   ***************************************************

2. Enumerate all primary outputs
   ***************************************************
   list_all_PO()
   {
      int i = 0;

      for(wptr po_wire: cktout){
           printf("%d-th PO wire: %s\n", i, po_wire->name.cstr());
           i++;
      }
   }
   **************************************************

3. Accessing the fanin node of a WIRE
   ****************************************************
   print_out_fanin_node_of_a_wire(wire)
      wptr   wire;
   {
      nptr fanin_node = wire->inode.front();
      printf("Fanin node is : %s\n", fanin_node->name.c_str());
   }
   ****************************************************

4. Accessing the fanout nodes of a WIRE:
   **************************************************************
   print_out_fanout_node_of_a_wire(wire)
      wptr    wire;
   {
      int    i = 0;

      for(nptr fanout_node: wire->onode){
        printf("%d-th fanout node is %s\n", i, fanout_node->name.c_str());
        i++;
      }
   }
   **************************************************************


5. Accessing the fanin wires of a NODE:
   **************************************************************
   print_out_fanin_wire_of_a_node(node)
      nptr    node;
   {
      int    i = 0;

      for(wptr fanin_wire: node->iwire){
          printf("%d-th fanin wire is %s\n", i, fanin_wire->name.c_str());
          i++;
      }
   }
   **************************************************************

6. Accessing the fanout wire of a NODE:
   **************************************************************
   print_out_fanout_wire_of_a_node(node)
      nptr   node;
   {
      fanout_wire = node->owire.front();
      printf("Fanin wire is : fanout_wire->name.c_str());
   }
   **************************************************************
