#include "cinn/poly/graph.h"

#include <gtest/gtest.h>

#include "cinn/ir/ir_operators.h"

namespace cinn {
namespace poly {

// Create a call.
Expr CreateCall(const std::string& name, const std::vector<Expr>& args) {
  auto expr = ir::Call::Make(Float(32), name, args, ir::Call::CallType::Halide);
  return expr;
}

Stage* CreateStage(const std::string& name, std::vector<Expr>& args, isl::set domain) {
  auto expr = CreateCall(name, args);
  return make_shared<Stage>(domain, expr);
}

TEST(CreateGraph, basic) {
  auto ctx = Context::Global().isl_ctx();
  // create call (for tensor);
  Var i("i"), j("j"), k("k");
  std::vector<Expr> args({Expr(i), Expr(j), Expr(k)});

  Var A_arr("A"), B_arr("B"), C_arr("C");
  Expr A_call = CreateCall("A", args);
  Expr B_call = CreateCall("B", args);
  Expr C_call = CreateCall("C", args);

  // A[] = B[] + 1
  Expr A_expr = ir::Store::Make(A_arr, Expr(1.f), Expr(i));
  Expr B_expr = ir::Store::Make(B_arr, A_call + 1.f, Expr(i));
  Expr C_expr = ir::Store::Make(C_arr, B_call + A_call, Expr(i));

  // create stages
  auto* A_stage = make_shared<Stage>(isl::set(ctx, "{ A[i,j,k]: 0<=i,j,k<100 }"), A_expr);
  auto* B_stage = make_shared<Stage>(isl::set(ctx, "{ B[i,j,k]: 0<=i,j,k<100 }"), B_expr);
  auto* C_stage = make_shared<Stage>(isl::set(ctx, "{ C[i,j,k]: 0<=i,j,k<100 }"), C_expr);

  auto graph = CreateGraph({A_stage, B_stage, C_stage});
  LOG(INFO) << "graph:\n" << graph->Visualize();
}

}  // namespace poly
}  // namespace cinn