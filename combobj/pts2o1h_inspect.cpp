#include <array>
#include <vector>
#include <string>
#include <boost/functional/hash.hpp>
#include "souffle/SouffleInterface.h"
#include "souffle/CompiledRamRecord.h"

using namespace souffle;

void error(std::string txt) {
	std::cerr << "error: " << txt << "\n";
	exit(1);
}

template <int o>
void unfold(RamDomain vec[], int vec_i, const tuple& t, int attr_i) {
}

template <int o, int fieldwidth, int ... fieldwidthrest>
void unfold(RamDomain vec[], int vec_i, const tuple& t, int attr_i) {
	if (fieldwidth == 0) {
		vec[vec_i++] = t[attr_i++];
		unfold<o, fieldwidthrest...>(vec, vec_i, t, attr_i);
	}
	else {
		ram::Tuple<RamDomain,fieldwidth?fieldwidth:1> rec = unpack<ram::Tuple<RamDomain,fieldwidth?fieldwidth:1>>(t[attr_i++]);
		for (size_t i = 0; i < fieldwidth; i++) vec[vec_i++] = rec[i];
		unfold<o, fieldwidthrest...>(vec, vec_i, t, attr_i);
	}
}

template <int first, int ... rest>
void unfold(RamDomain vec[], const tuple& t) {
	unfold<0, first, rest...>(vec, 0, t, 0);
}

size_t get_fact_hashval(size_t rel_hashval, RamDomain vec[], size_t arity) {
	size_t result = rel_hashval;
	boost::hash_combine(result, boost::hash_range(vec, vec+arity));
	return result;
}

int main(int argc, char **argv) {
	std::cout << CTX_LEN << " " << HCTX_LEN << "\n";
	if (SouffleProgram *prog = ProgramFactory::newInstance("pts2o1h_genclass")) {
		std::cout << "successfully loaded!\n";
		prog->loadAll(argv[1]);
		prog->run();
		//prog->printAll();
		const SymbolTable& progSymTable = prog->getSymbolTable();
		for (auto rel : prog->getAllRelations()) {
			std::cout << rel->getName() << " " << rel->getSignature() << "\n";
		}
		std::string rel_name = "VarPointsTo";
		int cnt = 0;
		if (Relation *rel = prog->getRelation(rel_name)) {
			size_t rel_hashval = std::hash<std::string>()(rel_name);
			size_t rel_arity = rel->getArity()-2+HCTX_LEN+CTX_LEN;
			for (auto &t : *rel) {
				RamDomain vec[rel_arity];
				unfold<HCTX_LEN, 0, CTX_LEN, 0>(vec, t);
				//ram::Tuple<RamDomain,HCTX_LEN> hctx = unpack<ram::Tuple<RamDomain,HCTX_LEN>>(t[0]);
				//for (size_t i = 0; i < HCTX_LEN; i++) vec[i] = hctx[i];
				//vec[HCTX_LEN] = t[1];
				//ram::Tuple<RamDomain,CTX_LEN> ctx = unpack<ram::Tuple<RamDomain,CTX_LEN>>(t[2]);
				//for (size_t i = 0; i < CTX_LEN; i++) vec[HCTX_LEN+i+1] = ctx[i];
				//vec[HCTX_LEN+CTX_LEN+1] = t[3];
				size_t fact_hashval = get_fact_hashval(rel_hashval, vec, rel_arity);
				if (cnt++ > 10) continue;
				std::cout << vec[0];
				for (size_t i = 1; i < rel_arity; i++)
					std::cout << " " << vec[i];
				std::cout << std::endl << std::hex << fact_hashval << std::dec << std::endl;
				std::cout << progSymTable.resolve(vec[0]) << std::endl;
				for (size_t i = 1; i < rel_arity; i++)
					std::cout << " " << progSymTable.resolve(vec[i]) << std::endl;
				std::cout << std::endl;
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
