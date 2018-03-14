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

struct key_type {
	size_t n;
	RamDomain * e;
	key_type(size_t n0, RamDomain* e0) : n(n0), e( new RamDomain[n] ) {
		for (int i = 0; i < n; i++)
			e[i] = e0[i];
	}
	key_type(const key_type& x) : n(x.n), e( new RamDomain[n] ) {
		n = x.n;
		for (int i = 0; i < n; i++)
			e[i] = x.e[i];
	}
	~key_type() {
		delete[] e;
	}
};

namespace std {
	template <> struct hash<key_type> {
		size_t operator()(const key_type &x) const {
			return boost::hash_range(x.e, x.e+x.n);
		}
	};
	template <> struct equal_to<key_type> {
		bool operator()(const key_type &x, const key_type &y) const {
			if (x.n != y.n) return false;
			for (int i = 0; i < x.n; i++)
				if (x.e[i] != y.e[i])
					return false;
			return true;
		}
	};
}

std::unordered_set<key_type> key_set;
std::unordered_map<key_type, size_t> map2hashval;

int main(int argc, char **argv) {
	// argv[1] : input
	// argv[2] : replace_file
	if (SouffleProgram *prog = ProgramFactory::newInstance("pts2o1h_genclass")) {
		std::cout << "pts2o1h_genclass successfully loaded!\n";
		prog->loadAll(argv[1]);
		prog->run();
		//prog->printAll();
		//for (auto rel : prog->getAllRelations()) {
		//	std::cout << rel->getName() << " " << rel->getSignature() << "\n";
		//}
		const SymbolTable& progSymTable = prog->getSymbolTable();
		init_hashval_of_all_symbols(progSymTable);
		size_t arity_proj = 3;
		std::string rel_name = "CallGraphEdge";
		if (Relation *rel = prog->getRelation(rel_name)) {
			size_t rel_hashval = std::hash<std::string>()(rel_name);
			size_t arity_attr = 4;
			size_t attr_width[arity_attr] = {2, 0, 2, 0};
			size_t dim_proj[arity_proj] = {3, 4, 5};
			size_t rel_arity = 0;
			for (size_t i = 0; i < arity_attr; i++)
				rel_arity += attr_width[i] == 0 ? 1 : attr_width[i];
			output_processor out((rel_name+".log").c_str());
			for (auto &t : *rel) {
				RamDomain vec[rel_arity];
				unfold(vec, t, arity_attr, attr_width);
				out.output_fact_info(vec, rel_arity);
				RamDomain proj_res[arity_proj];
				for (size_t i = 0; i < arity_proj; i++)
					proj_res[i] = vec[dim_proj[i]];
				key_type k(arity_proj, proj_res);
				key_set.insert(k);
			}
			std::cout << key_set.size() << std::endl;
		} 
		else 
			error("cannot find relation " + rel_name);
	}
	else {
		error("cannot find program oh");
	}
	return 0;
}
