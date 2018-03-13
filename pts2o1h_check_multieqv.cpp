#include <array>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include "souffle/SouffleInterface.h"
#include "souffle/CompiledRamRecord.h"

using namespace souffle;

void error(std::string txt) {
	std::cerr << "error: " << txt << "\n";
	exit(1);
}

void custom_unpack(RamDomain vec[], size_t& vec_i, int width, size_t attr_i, const tuple& t) {
	assert(1 <= width && width <= 3);
	switch (width) {
		case 1: {
			ram::Tuple<RamDomain,1> rec = unpack<ram::Tuple<RamDomain,1>>(t[attr_i]);
			vec[vec_i++] = rec[0];
		}
			break;
		case 2: {
			ram::Tuple<RamDomain,2> rec = unpack<ram::Tuple<RamDomain,2>>(t[attr_i]);
			vec[vec_i++] = rec[0];
			vec[vec_i++] = rec[1];
		}
			break;
		case 3: {
			ram::Tuple<RamDomain,3> rec = unpack<ram::Tuple<RamDomain,3>>(t[attr_i]);
			vec[vec_i++] = rec[0];
			vec[vec_i++] = rec[1];
			vec[vec_i++] = rec[2];
		}
			break;
	}
}

void unfold(RamDomain vec[], const tuple& t, size_t attr_width_n, size_t attr_width[]) {
	size_t vec_i = 0;
	for (size_t attr_i = 0; attr_i < attr_width_n; attr_i++) {
		size_t aw = attr_width[attr_i];
		if (aw == 0) {
			vec[vec_i++] = t[attr_i];
		}
		else {
			custom_unpack(vec, vec_i, aw, attr_i, t);
		}
	}
}

std::vector<size_t> numToHashval;

void init_hashval_of_all_symbols(const SymbolTable &symTable) {
	size_t n = symTable.size();
	numToHashval.resize(n);
	std::hash<std::string> str_hasher;
	for (size_t i = 0; i < n; i++)
		numToHashval[i] = str_hasher(symTable.resolve(i));
}

// TODO: obtain three containers in a class

class output_processor {
	FILE* outfile;
public:
	output_processor(const char * filename) {
		outfile = fopen(filename, "w");
	}
	void output_fact_info(RamDomain vec[], size_t rel_arity) {
		fprintf(outfile, "%d", (int)vec[0]);
		for (int i = 1; i < rel_arity; i++)
			fprintf(outfile, "\t%d", (int)vec[i]);
		fprintf(outfile, "\n");
	}
	~output_processor() {
		fclose(outfile);
	}
};

int main(int argc, char **argv) {
	// argv[1] : input
	// argv[2] : replace_file
	if (SouffleProgram *prog = ProgramFactory::newInstance("pts2o1h_genclass")) {
		std::cout << "pts2o1h_genclass successfully loaded!\n";
		prog->loadAll(argv[1]);
		prog->run();
		//prog->printAll();
		const SymbolTable& progSymTable = prog->getSymbolTable();
		init_hashval_of_all_symbols(progSymTable);
		//for (auto rel : prog->getAllRelations()) {
		//	std::cout << rel->getName() << " " << rel->getSignature() << "\n";
		//}
		std::string rel_name = "CallGraphEdge";
		if (Relation *rel = prog->getRelation(rel_name)) {
			size_t rel_hashval = std::hash<std::string>()(rel_name);
			size_t arity_ori = 4;
			size_t attr_width[arity_ori] = {2, 0, 2, 0};
			size_t rel_arity = 0;
			for (size_t i = 0; i < arity_ori; i++)
				rel_arity += attr_width[i] == 0 ? 1 : attr_width[i];
			output_processor out((rel_name+".log").c_str());
			for (auto &t : *rel) {
				RamDomain vec[rel_arity];
				unfold(vec, t, arity_ori, attr_width);
				out.output_fact_info(vec, rel_arity);
			}
		} 
		else 
			error("cannot find relation " + rel_name);
	}
	else {
		error("cannot find program oh");
	}
	return 0;
}
