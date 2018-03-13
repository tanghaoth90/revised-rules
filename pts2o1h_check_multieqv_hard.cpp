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

template <int fieldwidth, int ... fieldwidthrest>
void unfold(RamDomain vec[], const tuple& t) {
	unfold<0, fieldwidth, fieldwidthrest...>(vec, 0, t, 0);
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

std::unordered_map<RamDomain, size_t> mp; // element -> xor (hash value)
std::unordered_set<size_t> hset; // hash value set
std::unordered_multimap<size_t, RamDomain> h2s; // hash value -> element
std::unordered_map<RamDomain, size_t> counter; // # of facts that the element appears

void init_mapping_tables() {
	mp.clear();
	hset.clear();
	h2s.clear();
	counter.clear();
}

size_t get_fact_hashval(size_t rel_hashval, RamDomain vec[], size_t arity, size_t blockpos = 1<<30) {
	size_t vec_hash[arity];
	for (size_t i = 0; i < arity; i++) 
		if (i == blockpos)
			vec_hash[i] = 0; // 0 as the placeholder hash value for blocked element
		else
			vec_hash[i] = numToHashval[vec[i]];
	size_t result = rel_hashval; // consider relation name
	size_t rg_r = boost::hash_range(vec_hash, vec_hash+arity);
	boost::hash_combine(result, rg_r); 
	return result;
}

void process_fact(size_t rel_hashval, RamDomain vec[], size_t arity, const bool comb_dim[]) {
	for (size_t i = 0; i < arity; i++) 
		if (comb_dim[i]) {
			RamDomain bval = vec[i];
			mp[bval] = mp[bval] ^ get_fact_hashval(rel_hashval, vec, arity, i);
			if (counter.count(bval) == 0)
				counter[bval] = 1;
			else
				counter[bval] = counter[bval] + 1;
		}
}
// TODO: counter and output info in a class
int global_counter = 0;

void init_counter() {
	global_counter = 0;
}

void output_fact_info(RamDomain vec[], size_t rel_arity, const SymbolTable& progSymTable, size_t fact_hashval) {
	if (global_counter++ > 10) return;
	std::cout << vec[0];
	for (size_t i = 1; i < rel_arity; i++)
		std::cout << " " << vec[i];
	std::cout << std::endl << std::hex << fact_hashval << std::dec << std::endl;
	std::cout << progSymTable.resolve(vec[0]) << std::endl;
	for (size_t i = 1; i < rel_arity; i++)
		std::cout << " " << progSymTable.resolve(vec[i]) << std::endl;
	std::cout << std::endl;
}

int main(int argc, char **argv) {
	// argv[1] : input
	// argv[2] : replace_file
	// argv[3] : limitation
	std::cout << CTX_LEN << " " << HCTX_LEN << "\n";
	if (SouffleProgram *prog = ProgramFactory::newInstance("pts2o1h_genclass")) {
		std::cout << "successfully loaded!\n";
		prog->loadAll(argv[1]);
		prog->run();
		//prog->printAll();
		const SymbolTable& progSymTable = prog->getSymbolTable();
		init_hashval_of_all_symbols(progSymTable);
		for (auto rel : prog->getAllRelations()) {
			std::cout << rel->getName() << " " << rel->getSignature() << "\n";
		}
		init_mapping_tables();
		std::string rel_name;
		rel_name = "VarPointsTo";
		init_counter();
		FILE* outfile_varpointsto = fopen("VarPointsTo.log", "w");
		if (Relation *rel = prog->getRelation(rel_name)) {
			const bool comb_dim[] = {true, true, true, true, true};
			size_t rel_hashval = std::hash<std::string>()(rel_name);
			size_t rel_arity = rel->getArity()-2+HCTX_LEN+CTX_LEN;
			for (auto &t : *rel) {
				RamDomain vec[rel_arity];
				unfold<HCTX_LEN, 0, CTX_LEN, 0>(vec, t);
				fprintf(outfile_varpointsto, "%d", (int)vec[0]);
				for (int i = 1; i < rel_arity; i++)
					fprintf(outfile_varpointsto, " %d", (int)vec[i]);
				fprintf(outfile_varpointsto, "\n");
			}
		} 
		else 
			error("cannot find relation " + rel_name);
		fclose(outfile_varpointsto);
		rel_name = "CallGraphEdge";
		init_counter();
		FILE* outfile_callgraphedge = fopen("CallGraphEdge.log", "w");
		if (Relation *rel = prog->getRelation(rel_name)) {
			const bool comb_dim[] = {true, true, true, true, true};
			size_t rel_hashval = std::hash<std::string>()(rel_name);
			size_t rel_arity = rel->getArity()-1+CTX_LEN;
			for (auto &t : *rel) {
				RamDomain vec[rel_arity];
				unfold<CTX_LEN, 0, CTX_LEN, 0>(vec, t);
				fprintf(outfile_callgraphedge, "%d", (int)vec[0]);
				for (int i = 1; i < rel_arity; i++)
					fprintf(outfile_callgraphedge, " %d", (int)vec[i]);
				fprintf(outfile_callgraphedge, "\n");
			}
		}
		else 
			error("cannot find relation " + rel_name);
		fclose(outfile_callgraphedge);
	}
	else {
		error("cannot find program oh");
	}
	return 0;
}
