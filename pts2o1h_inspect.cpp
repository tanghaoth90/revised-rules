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
	void output_i2s(RamDomain index, std::string symbol) {
		fprintf(outfile, "%d\t%s\n", (int)index, symbol.c_str());
	}
	~output_processor() {
		fclose(outfile);
	}
};

std::unordered_set<RamDomain> used_index;

void output_index2symbol(const SymbolTable &symTable, const char * filename) {
	output_processor op(filename);
	size_t n = symTable.size();
	for (auto it : used_index)
		op.output_i2s(it, symTable.resolve(it));
}

void process_relation(std::string rel_name, size_t arity_attr, size_t attr_width[], SouffleProgram *prog) {
	if (Relation *rel = prog->getRelation(rel_name)) {
		size_t rel_hashval = std::hash<std::string>()(rel_name);
		size_t rel_arity = 0;
		for (size_t i = 0; i < arity_attr; i++)
			rel_arity += attr_width[i] == 0 ? 1 : attr_width[i];
		output_processor out((rel_name+".csv").c_str());
		for (auto &t : *rel) {
			RamDomain vec[rel_arity];
			unfold(vec, t, arity_attr, attr_width);
			out.output_fact_info(vec, rel_arity);
			for (size_t i = 0; i < rel_arity; i++)
				used_index.insert(vec[i]);
		}
	} 
	else 
		error("cannot find relation " + rel_name);
}

int main(int argc, char **argv) {
	// TODO process argv using getopt(_long) function to get "genclass (or other names)", "-F", "-D", etc.
	if (SouffleProgram *prog = ProgramFactory::newInstance("pts2o1h_genclass")) {
		std::cout << "pts2o1h_genclass successfully loaded!\n";
		prog->loadAll(argv[1]); // -D
		prog->run();
		//prog->printAll();
		//for (auto rel : prog->getAllRelations()) {
		//	std::cout << rel->getName() << " " << rel->getSignature() << "\n";
		//}
		used_index.clear();
		{
		size_t attr_width[4] = {2, 0, 2, 0};
		process_relation("CallGraphEdge", 4, attr_width, prog);
		}
		{
		size_t attr_width[4] = {1, 0, 2, 0};
		process_relation("ThrowPointsTo", 4, attr_width, prog);
		}
		/*
		{
		size_t attr_width[2] = {0, 0};
		process_relation("OptVirtualMethodInvocationBase", 2, attr_width, prog);
		}
		{
		size_t attr_width[4] = {1, 0, 2, 0};
		process_relation("VarPointsTo", 4, attr_width, prog);
		}
		{
		size_t attr_width[2] = {0, 0};
		process_relation("Value_Type", 2, attr_width, prog);
		}
		*/
		const SymbolTable& progSymTable = prog->getSymbolTable();
		output_index2symbol(progSymTable, "index2symbol.csv");
		delete prog;
	}
	else {
		error("cannot find program"); // TODO add name of genclass
		exit(1);
	}
	return 0;
}
