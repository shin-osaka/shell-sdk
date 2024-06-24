



#ifndef COMMON_LINUX_MODULE_H__
#define COMMON_LINUX_MODULE_H__

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "common/symbol_data.h"
#include "common/using_std_string.h"
#include "google_breakpad/common/breakpad_types.h"

namespace google_breakpad {

using std::set;
using std::vector;
using std::map;

class Module {
 public:
  typedef uint64_t Address;
  struct File;
  struct Function;
  struct Line;
  struct Extern;


  struct File {
    explicit File(const string &name_input) : name(name_input), source_id(0) {}

    const string name;

    int source_id;
  };

  struct Function {
    Function(const string &name_input, const Address &address_input) :
        name(name_input), address(address_input), size(0), parameter_size(0) {}

    static bool CompareByAddress(const Function *x, const Function *y) {
      return x->address < y->address;
    }

    const string name;

    const Address address;
    Address size;

    Address parameter_size;

    vector<Line> lines;
  };

  struct Line {
    static bool CompareByAddress(const Module::Line &x, const Module::Line &y) {
      return x.address < y.address;
    }

    Address address, size;    // The address and size of the line's code.
    File *file;                // The source file.
    int number;                // The source line number.
  };

  struct Extern {
    explicit Extern(const Address &address_input) : address(address_input) {}
    const Address address;
    string name;
  };

  typedef map<string, string> RuleMap;

  typedef map<Address, RuleMap> RuleChangeMap;

  struct StackFrameEntry {
    Address address, size;

    RuleMap initial_rules;

    RuleChangeMap rule_changes;
  };

  struct FunctionCompare {
    bool operator() (const Function *lhs,
                     const Function *rhs) const {
      if (lhs->address == rhs->address)
        return lhs->name < rhs->name;
      return lhs->address < rhs->address;
    }
  };

  struct ExternCompare {
    bool operator() (const Extern *lhs,
                     const Extern *rhs) const {
      return lhs->address < rhs->address;
    }
  };

  Module(const string &name, const string &os, const string &architecture,
         const string &id);
  ~Module();

  void SetLoadAddress(Address load_address);

  void AddFunction(Function *function);

  void AddFunctions(vector<Function *>::iterator begin,
                    vector<Function *>::iterator end);

  void AddStackFrameEntry(StackFrameEntry *stack_frame_entry);

  void AddExtern(Extern *ext);

  File *FindFile(const string &name);
  File *FindFile(const char *name);

  File *FindExistingFile(const string &name);

  void GetFunctions(vector<Function *> *vec, vector<Function *>::iterator i);

  void GetExterns(vector<Extern *> *vec, vector<Extern *>::iterator i);

  void GetFiles(vector<File *> *vec);

  void GetStackFrameEntries(vector<StackFrameEntry *> *vec) const;

  void AssignSourceIds();

  bool Write(std::ostream &stream, SymbolData symbol_data);

  string name() const { return name_; }
  string os() const { return os_; }
  string architecture() const { return architecture_; }
  string identifier() const { return id_; }

 private:
  static bool ReportError();

  static bool WriteRuleMap(const RuleMap &rule_map, std::ostream &stream);

  string name_, os_, architecture_, id_;

  Address load_address_;

  struct CompareStringPtrs {
    bool operator()(const string *x, const string *y) const { return *x < *y; }
  };

  typedef map<const string *, File *, CompareStringPtrs> FileByNameMap;

  typedef set<Function *, FunctionCompare> FunctionSet;

  typedef set<Extern *, ExternCompare> ExternSet;

  FileByNameMap files_;    // This module's source files.
  FunctionSet functions_;  // This module's functions.

  vector<StackFrameEntry *> stack_frame_entries_;

  ExternSet externs_;
};

}  // namespace google_breakpad

#endif  // COMMON_LINUX_MODULE_H__
