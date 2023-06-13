/**********************************************************************/
/*           Read circuit and parse                                   */
/*                                                                    */
/*           Author: Bing-Chen (Benson) Wu                            */
/*           last update : 01/21/2018                                 */
/**********************************************************************/

#include "atpg.h"

/* convert the name into integer to index the hashtable */
int ATPG::hashcode(const string &name) {
  int i = 0, j = 0;

  for (auto pos = name.cbegin(), end = name.cend(); (pos != end) && (j < 8); ++pos) {
    i = i * 10 + (*pos - '0');
    ++j;
  }

  i = i % HASHSIZE;
  return i < 0 ? i + HASHSIZE : i;
}/* end of hashcode */

/* wfind checks the existence of a wire
 * return the wire pointer if it exists; otherwise return a NULL pointer
 */
ATPG::wptr ATPG::wfind(const string &name) {
  int hash_idx = hashcode(name);

  /* search the forward_list at hast_nlist[code],
     using iterator to visit the elements within the forward_list    */
  for (const auto &pos : hash_wlist[hash_idx]) {
    if (name == pos->name) return pos.get(); // "return *pos" would return a unique_ptr obj, that's wrong.
  }
  return (nullptr);
}/* end of wfind */

/* nfind checks the existence of a node
 * return the node pointer if it exists; otherwise return a NULL pointer
 */
ATPG::nptr ATPG::nfind(const string &name) {
  int hash_idx = hashcode(name);

  for (const auto &pos : hash_nlist[hash_idx]) {
    if (name == pos->name) return pos.get(); // "return *pos" would return a unique_ptr obj, that's wrong.
  }
  return (nullptr);
}/* end of nfind */

/* get wire obj and return the wire pointer */
ATPG::wptr ATPG::getwire(const string &wirename) {
  wptr w = wfind(wirename);
  if (w != nullptr) { return (w); }

  /* allocate new wire from free storage */
  wptr_s wtemp(new(nothrow) WIRE);
  if (wtemp == nullptr) { error("No more room!"); }

  /* initialize wire entries */
  wtemp->name = wirename;
  int hash_no = hashcode(wirename);
  hash_wlist[hash_no].push_front(move(wtemp)); // push into the hash table
  return hash_wlist[hash_no].front().get();
}/* end of getwire */

/* get node obj and return the node pointer */
ATPG::nptr ATPG::getnode(const string &nodename) {
  nptr n = nfind(nodename);
  if (n != nullptr) { return (n); }

  /* allocate new node from free storage */
  nptr_s ntemp(new(nothrow) NODE);
  if (ntemp == nullptr) { error("No more room!"); }

  /* initialize node entries */
  ntemp->name = nodename;
  int hash_no = hashcode(nodename);
  hash_nlist[hash_no].push_front(move(ntemp)); // push into the hash table
  return hash_nlist[hash_no].front().get();
}/* end of getnode */

/* new gate */
void ATPG::newgate() {
  nptr n;
  wptr w;
  int i;

  if ((targc < 5) || (targv[targc - 2] != ";")) error("Bad Gate Record");
  n = getnode(targv[0]);
  n->type = FindType(targv[1]);
  i = 2;
  /* connect the iwire and owire */
  while (i < targc) {
    if (targv[i] == ";") break;
    w = getwire(targv[i]);
    //w->onode.push_back(n);
    n->iwire.push_back(w);
    i++;
  }
  if (i == 2 || ((i + 2) != targc)) error("Bad Gate Record");
  i++;
  w = getwire(targv[i]);
  //w->inode.push_back(n);
  n->owire.push_back(w);
}/* end of newgate */

/* each PO is treated like a wire */
/* dummy PO gate will be added later, see init_flist.cpp */
void ATPG::set_output() {
  wptr w;
  int i;

  for (i = 1; i < targc; i++) {
    w = getwire(targv[i]);
    for (auto pos : cktout) {
      if (w == pos) {
        fprintf(stderr, "net %s is declared again as output around line %d\n", w->name.c_str(), lineno);
        exit(EXIT_FAILURE);
      }
    }
    w->set_output();
    cktout.push_back(w);
  }
}/* end of set_output */

/* each PI is treated like a wire */
/* dummy PI gate will be added later, see init_flist.c */
void ATPG::set_input(const bool &pori) {
  wptr w;
  int i;

  for (i = 1; i < targc; i++) {
    w = getwire(targv[i]);
    for (auto pos : cktin) {
      if (w == pos) {
        fprintf(stderr, "net %s is declared again as input around line %d\n", w->name.c_str(), lineno);
        exit(EXIT_FAILURE);
      }
    }
    w->set_input();
    cktin.push_back(w);
  }
}/* end of set_input */

/* parse input line into tokens, filling up targv and setting targc */
void ATPG::parse_line(const string &line) {
  for (auto &i : targv) {
    i.clear();
  }
  if (!line.empty()) {
    targc = 1;
    for (char pos : line) {
      if (pos <= ' ') {
        targc++;
        continue;
      }
      targv[targc - 1].push_back(pos);
    }
  } else {
    targc = 0;
  }
}/* end of parse_line */

void ATPG::input(const string &infile) {
  string line;
  filename = infile;
  ifstream file(filename, std::ifstream::in); // open the input vectors' file
  if (!file) { // if the ifstream obj does not exist, fail to open the file
    fprintf(stderr, "Cannot open input file %s\n", filename.c_str());
    exit(EXIT_FAILURE);
  }
  while (!file.eof() && !file.bad()) {
    getline(file, line); // get a line from the file
    lineno++;
    parse_line(line);
    //cout << targc << endl;
    for (int i = 0; !targv[i].empty(); i++) {
      //cout << "targv[" << i <<"]: " << targv[i] << endl;
    }
    if (targv[0].empty()) continue;
    if (targv[0] == "name") {
      if (targc != 2) {
        //cout << targc << endl;
        error("Wrong Input Format!");
      }
      continue;
    }
    switch (targv[0][0]) {
      case '#':
        break;

      case 'D':
        debug = 1 - debug;
        break;

      case 'g':
        newgate();
        break;

      case 'i':
        set_input(false);
        break;

      case 'p':
        set_input(true);
        break;

      case 'o':
        set_output();
        break;

      case 'n':
        set_output();
        break;

      default:
        fprintf(stderr, "Unrecognized command around line %d in file %s\n", lineno, filename.c_str());
        break;
    }
  }

  int ncktnode = 0;
  int ncktwire = 0;
  for (int i = 0; i < HASHSIZE; i++) {
    ncktnode += distance(hash_nlist[i].begin(), hash_nlist[i].end());
    ncktwire += distance(hash_wlist[i].begin(), hash_wlist[i].end());
  }

  file.close();
  create_structure();
  fprintf(stdout, "\n");
  fprintf(stdout, "#Circuit Summary:\n");
  fprintf(stdout, "#---------------\n");
  fprintf(stdout, "#number of inputs = %d\n", int(cktin.size()));
  fprintf(stdout, "#number of outputs = %d\n", int(cktout.size()));
  fprintf(stdout, "#number of gates = %d\n", ncktnode);
  fprintf(stdout, "#number of wires = %d\n", ncktwire);
  if (debug) display_circuit();
  //display_circuit();
}/* end of input */

void ATPG::create_structure() {
  nptr n;
  int i;

  /*walk through every node in the circuit  */
  for (i = 0; i < HASHSIZE; i++) {
    for (const auto &pos : hash_nlist[i]) { // for each node n in the circuit
      n = pos.get();

      for (wptr w: n->iwire) { //insert node n into the onode of its iwires
        w->onode.push_back(n);
      }

      for (wptr w: n->owire) { //insert node n into the inode of its owires
        w->inode.push_back(n);
      }// for w
    }// for n
  }// for i
}

int ATPG::FindType(const string &gatetype) {
  if (gatetype == "and") return (AND);
  if (gatetype == "AND") return (AND);
  if (gatetype == "nand") return (NAND);
  if (gatetype == "NAND") return (NAND);
  if (gatetype == "or") return (OR);
  if (gatetype == "OR") return (OR);
  if (gatetype == "nor") return (NOR);
  if (gatetype == "NOR") return (NOR);
  if (gatetype == "not") {
    if (targc != 5) error("Bad Gate Record");
    return (NOT);
  }
  if (gatetype == "NOT") {
    if (targc != 5) error("Bad Gate Record");
    return (NOT);
  }
  if (gatetype == "buf") {
    if (targc != 5) error("Bad Gate Record");
    return (BUF);
  }
  if (gatetype == "xor") {
    if (targc != 6) error("Bad Gate Record");
    return (XOR);
  }
  if (gatetype == "eqv") {
    if (targc != 6) error("Bad Gate Record");
    return (EQV);
  }
  error("unreconizable gate type");
}/* end of FindType */

/*
* print error message and die
*/
void ATPG::error(const string &message) {
  fprintf(stderr, "%s around line %d in file %s\n", message.c_str(), lineno, filename.c_str());
  exit(EXIT_FAILURE);
}/* end of error */

void ATPG::timer(FILE *file, const string &mesg1) {
  double t_meas = (double) clock();
  if (mesg1 == "START") {
    StartTime = t_meas;
    LastTime = StartTime;
    return;
  }
  fprintf(file, "#atpg: cputime %s %s: %.1fs %.1fs\n", mesg1.c_str(), filename.c_str(),
          (t_meas - LastTime) / CLOCKS_PER_SEC, (t_meas - StartTime) / CLOCKS_PER_SEC);
  LastTime = t_meas;
}/* end of timer */

/* this function is active only when the debug variable is turned on */
void ATPG::display_circuit() {
  int i;

  fprintf(stdout, "\n");
  for (i = 0; i < HASHSIZE; i++) {
    for (const auto &pos : hash_nlist[i]) {
      fprintf(stdout, "%s ", pos->name.c_str());
      switch (pos->type) {
        case AND :
          fprintf(stdout, "and ");
          break;
        case NAND :
          fprintf(stdout, "nand ");
          break;
        case OR :
          fprintf(stdout, "or ");
          break;
        case NOR :
          fprintf(stdout, "nor ");
          break;
        case BUF :
          fprintf(stdout, "buf ");
          break;
        case NOT :
          fprintf(stdout, "not ");
          break;
        case XOR :
          fprintf(stdout, "xor ");
          break;
        case EQV :
          fprintf(stdout, "eqv ");
          break;
      }
      for (auto pos1 = pos->iwire.cbegin(), end1 = pos->iwire.cend(); pos1 != end1; ++pos1) {
        fprintf(stdout, "%s ", (*pos1)->name.c_str());
      }
      fprintf(stdout, "; ");
      for (auto pos1 = pos->owire.cbegin(), end1 = pos->owire.cend(); pos1 != end1; ++pos1) {
        fprintf(stdout, "%s\n", (*pos1)->name.c_str());
      }
    }
  }
  fprintf(stdout, "i ");
  for (auto pos : cktin) {
    fprintf(stdout, "%s ", pos->name.c_str());
  }
  fprintf(stdout, "\n");
  fprintf(stdout, "o ");
  for (auto pos : cktout) {
    fprintf(stdout, "%s ", pos->name.c_str());
  }
  fprintf(stdout, "\n");

  fprintf(stdout, "\n");
  for (i = 0; i < HASHSIZE; i++) {
    for (const auto &pos : hash_wlist[i]) {
      fprintf(stdout, "%s ", pos->name.c_str());
      for (auto pos1 = pos->inode.cbegin(), end1 = pos->inode.cend(); pos1 != end1; ++pos1) {
        fprintf(stdout, "%s ", (*pos1)->name.c_str());
      }
      fprintf(stdout, ";");
      for (auto pos1 = pos->onode.cbegin(), end1 = pos->onode.cend(); pos1 != end1; ++pos1) {
        fprintf(stdout, "%s ", (*pos1)->name.c_str());
      }
      fprintf(stdout, "\n");
    }
  }
} /* end of display_circuit */


//  read and parse vector file
void ATPG::read_vectors(const string &vetFile) {
  string t, vec;
  size_t i;

  ifstream file(vetFile, std::ifstream::in); // open the input vectors' file
  if (!file) { // if the ifstream obj does not exist, fail to open the file
    fprintf(stderr, "File %s could not be opened\n", vetFile.c_str());
    exit(EXIT_FAILURE);
  }

  while (!file.eof() && !file.bad()) {
    getline(file, t); // get a line from the file
    if (t[0] != 'T') continue; // if this line is not a vector, ignore it
    else {
      vec.clear();
      for (char c: t) {
        if (c == 'T') continue;  // ignore "T"
        if (c == '\'') continue; // ignore "'"
        if (c == ' ') continue;  // ignore " "
        vec.push_back(c);
      }
      //cout << "Before erase: " << t << endl;
      //cout << "After erase: " << vec << endl;
      vectors.push_back(vec); // append the vectors
    }
  }
  file.close(); // close the file
}
