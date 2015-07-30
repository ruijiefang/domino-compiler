#include <iostream>
#include <set>
#include <string>
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "run_pass.h"
#include "func_transform_handler.cc"
#include "packet_variable_census.h"
#include "state_var_decl_handler.h"
#include "packet_decl_creator.h"
#include "if_conversion_handler.h"

using namespace clang::tooling;

static llvm::cl::OptionCategory if_conversion(""
"This allows us to rewrite if statements into ternary operators"
" and recursively get rid of all branches, from the innermost to"
" the outermost ones.");

int main(int argc, const char **argv) {
  // Set up parser options for refactoring tool
  CommonOptionsParser op(argc, argv, if_conversion);

  // Run passes, chaining results if required
  const auto state_vars     = run_pass<StateVarDeclHandler,
                                      std::string>
                                      (op, clang::ast_matchers::decl().bind("decl"));

  const auto packet_var_set = run_pass<PacketVariableCensus,
                                      std::set<std::string>>
                                      (op, clang::ast_matchers::decl().bind("decl"));

  const auto prog_decl_pair = run_pass<FuncTransformHandler<IfConversionHandler>,
                                      std::pair<std::string, std::vector<std::string>>>
                                      (op, clang::ast_matchers::functionDecl().bind("functionDecl"),
                                      packet_var_set);

  const auto packet_decls   = run_pass<PacketDeclCreator,
                                      std::string>
                                      (op, clang::ast_matchers::decl().bind("decl"),
                                      prog_decl_pair.second);

  // Print out outputs in sequence
  std::cout << state_vars << std::endl
            << packet_decls << std::endl
            << prog_decl_pair.first << std::endl;

  return 0;
}