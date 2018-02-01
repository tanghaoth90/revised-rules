#include <array>
#include <vector>
#include <string>
#include "souffle/SouffleInterface.h"

using namespace souffle;

void error(std::string txt) {
	std::cerr << "error: " << txt << "\n";
	exit(1);
}

std::unordered_map<std::string, std::string[3]> ctx_mapping, hctx_mapping;

int main(int argc, char **argv) {
	if (SouffleProgram *prog = ProgramFactory::newInstance("pts2o1h_genclass")) {
		std::cout << "successfully loaded!\n";
		prog->loadAll(argv[1]);
		prog->run();
		//prog->printAll();
		for (auto rel : prog->getAllRelations()) {
			std::cout << rel->getName() << " " << rel->getSignature() << "\n";
		}
		size_t arity_ctx, arity_hctx;
		if (Relation *rel_unfoldedcontext = prog->getRelation("UnfoldedContext")) {
			arity_ctx = rel_unfoldedcontext->getArity();
			for (auto &t : *rel_unfoldedcontext) {
				size_t ctx_id = t[0];
				std::string ctx_ss[arity_ctx-1];
				for (size_t i = 1; i < arity_ctx; i++) {
					ctx_ss[i-1] = t[i];
				}
			}
		}
		if (Relation *rel_unfoldedhcontext = prog->getRelation("UnfoldedHContext")) {
			arity_hctx = rel_unfoldedhcontext->getArity();
			for (auto &t : *rel_unfoldedhcontext) {
				size_t hctx_id = t[0];
				std::string hctx_ss[arity_hctx-1];
				for (size_t i = 1; i < arity_hctx; i++) {
					hctx_ss[i-1] = t[i];
				}
			}
		}
		if (Relation *rel_varpointsto = prog->getRelation("VarPointsTo")) {
			size_t arity_varpointsto = rel_varpointsto->getArity();
			std::cout << "arity of varpointsto: " << arity_varpointsto << "\n";
			for (auto &t : *rel_varpointsto) {
				std::string fact[arity_varpointsto+arity_ctx+arity_hctx-2];
				size_t hctx_id = t[0], ctx_id = t[2];
				//std::string myvalue = rel_varpointsto->getSymbolTable().resolve(t[1]), myvar = rel_varpointsto->getSymbolTable().resolve(t[3]);
				fact[arity_hctx] = rel_varpointsto->getSymbolTable().resolve(t[1]);
				fact[arity_varpointsto+arity_ctx+arity_hctx-3] = rel_varpointsto->getSymbolTable().resolve(t[3]);
				//std::cout << hctx_id << "-" << myvalue << "-" << ctx_id << "-" << myvar << "\n";
				// hash: consider the relation name!!
			}
		} else {
			error("cannot find relation VarPointsTo");
		}
	}
	else {
		error("cannot find program oh");
	}
	return 0;
}
