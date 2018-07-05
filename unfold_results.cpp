#include <array>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <boost/functional/hash.hpp>
#include "souffle/SouffleInterface.h"
//#include "souffle/CompiledRamRecord.h"
#include "souffle/CompiledRecord.h"

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
	static std::string path_prefix;
	static void set_path_prefix(std::string prefix) {
		path_prefix = prefix;
	}
	output_processor(std::string filename) {
		outfile = fopen((path_prefix+"/"+filename).c_str(), "w");
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

std::string output_processor::path_prefix(".");

std::unordered_set<RamDomain> used_index;

void output_index2symbol(const SymbolTable &symTable, std::string filename) {
	output_processor op(filename);
	size_t n = symTable.size();
	for (auto it : used_index)
		op.output_i2s(it, symTable.resolve(it));
	std::cout << "index2symbol finished\n";
}

void process_relation(std::string rel_name, size_t arity_attr, size_t attr_width[], SouffleProgram *prog) {
	if (Relation *rel = prog->getRelation(rel_name)) {
		size_t rel_hashval = std::hash<std::string>()(rel_name);
		size_t rel_arity = 0;
		for (size_t i = 0; i < arity_attr; i++)
			rel_arity += attr_width[i] == 0 ? 1 : attr_width[i];
		output_processor out(rel_name+".csv");
		for (auto &t : *rel) {
			RamDomain vec[rel_arity];
			unfold(vec, t, arity_attr, attr_width);
			out.output_fact_info(vec, rel_arity);
			for (size_t i = 0; i < rel_arity; i++)
				used_index.insert(vec[i]);
		}
		std::cout << rel_name << " finished\n";
	} 
	else 
		error("cannot find relation " + rel_name);
}

class rel_schema_processor {
	FILE* schema_file;
public:
	rel_schema_processor(char * schema_file_name) {
		schema_file = fopen(schema_file_name, "r");
	}
	size_t* get_one_schema(std::string& rel_name, size_t& arity) {
		static char tmp_st[20];
		if (fscanf(schema_file, "%s", tmp_st) == EOF) return NULL;
		rel_name = tmp_st;
		if (fscanf(schema_file, "%lu", &arity) != 1) 
			error("schema wrong in relation " + rel_name);
		size_t* result = new size_t[arity];
		for (size_t i = 0; i < arity; i++) {
			if (fscanf(schema_file, "%s", tmp_st) != 1 || strlen(tmp_st) != 1)
				error("schema wrong in relation " + rel_name + " (only C/H/0 is allowed)");
			switch (tmp_st[0]) {
				case 'C': result[i] = CTX_LEN; break;
				case 'H': result[i] = HCTX_LEN; break;
				case '0': result[i] = 0; break;
				default: error("schema wrong in relation " + rel_name + " (only C/H/0 is allowed)");
			}
		}
		return result;
	}
	~rel_schema_processor() {
		fclose(schema_file);
	}
};

int main(int argc, char **argv) {
	// TODO process argv using getopt(_long) function to get "genclass (or other names)", "-F", "-D", etc.
	if (SouffleProgram *prog = ProgramFactory::newInstance(argv[1])) {
		std::cout << argv[1] << " successfully loaded!\n";
		prog->loadAll(argv[2]); // -D
		output_processor::set_path_prefix(argv[3]);
		prog->run();
		//prog->printAll();
		//for (auto rel : prog->getAllRelations()) {
		//	std::cout << rel->getName() << " " << rel->getSignature() << "\n";
		//}
		used_index.clear();
		rel_schema_processor rsp(argv[4]);
		size_t* attr_width;
		size_t arity_attr;
		std::string rel_name;
		while ((attr_width = rsp.get_one_schema(rel_name, arity_attr)) != NULL) {
			process_relation(rel_name, arity_attr, attr_width, prog);
			delete[] attr_width;
		}
		const SymbolTable& progSymTable = prog->getSymbolTable();
		output_index2symbol(progSymTable, "index2symbol.csv");
		delete prog;
	}
	else {
		error(std::string("cannot find program ")+argv[1]);
		exit(1);
	}
	return 0;
}
