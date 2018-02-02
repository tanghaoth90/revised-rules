#include <array>
#include <vector>
#include <string>
#include "souffle/SouffleInterface.h"
#include "souffle/CompiledRamRecord.h"

using namespace souffle;

void error(std::string txt) {
	std::cerr << "error: " << txt << "\n";
	exit(1);
}

int main(int argc, char **argv) {
	std::cout << CTX_LEN << " " << HCTX_LEN << "\n";
	if (SouffleProgram *prog = ProgramFactory::newInstance("pts2o1h_genclass")) {
		std::cout << "successfully loaded!\n";
		prog->loadAll(argv[1]);
		prog->run();
		const SymbolTable& progSymTable = prog->getSymbolTable();
		//prog->printAll();
		for (auto rel : prog->getAllRelations()) {
			std::cout << rel->getName() << " " << rel->getSignature() << "\n";
		}
		if (Relation *rel_varpointsto = prog->getRelation("VarPointsTo")) {
			//size_t arity_varpointsto = rel_varpointsto->getArity();
			//std::cout << "arity of varpointsto: " << arity_varpointsto << "\n";
			size_t cnt = 0;
			for (auto &t : *rel_varpointsto) {
				std::string fact[HCTX_LEN+CTX_LEN+2];
				ram::Tuple<RamDomain,HCTX_LEN> hctx = unpack<ram::Tuple<RamDomain,HCTX_LEN>>(t[0]);
				for (size_t i = 0; i < HCTX_LEN; i++)
					fact[i] = progSymTable.resolve(hctx[i]);
				fact[HCTX_LEN] = progSymTable.resolve(t[1]);
				ram::Tuple<RamDomain,CTX_LEN> ctx = unpack<ram::Tuple<RamDomain,CTX_LEN>>(t[2]);
				for (size_t i = 0; i < CTX_LEN; i++)
					fact[HCTX_LEN+i+1] = progSymTable.resolve(ctx[i]);
				fact[HCTX_LEN+CTX_LEN+1] = progSymTable.resolve(t[3]);
				//std::cout << hctx_id << "-" << myvalue << "-" << ctx_id << "-" << myvar << "\n";
				// hash: consider the relation name!!
				//fact[0] = unpack
				if (cnt++ < 10) {
					std::cout << fact[0] << std::endl;
					for (size_t i = 1; i < HCTX_LEN+CTX_LEN+2; i++)
						std::cout << " " << fact[i] << std::endl;
					std::cout << std::endl;
				}
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
