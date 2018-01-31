#include <array>
#include <vector>
#include <string>
#include "souffle/SouffleInterface.h"

using namespace souffle;

void error(std::string txt) {
	std::cerr << "error: " << txt << "\n";
	exit(1);
}

int main(int argc, char **argv) {
	if (SouffleProgram *prog = ProgramFactory::newInstance("pts2o1h_genclass")) {
		std::cout << "successfully loaded!\n";
		prog->loadAll(argv[1]);
		prog->run();
		//prog->printAll();
		/*
		if (Relation *path = prog->getRelation("Instruction_Method")) {
			for (auto &output : *path) {
				std::string s1, s2;
				output >> s1 >> s2;
				std::cout << s1 << "-" << s2 << "\n";
				break;
			}
		} else {
			error("cannot find relation Instruction_Method");
		}
		*/
		for (auto rel : prog->getAllRelations()) {
			std::cout << rel->getName() << " " << rel->getSignature() << "\n";
		}
		if (Relation *rel_varpointsto = prog->getRelation("VarPointsTo")) {
			for (auto &t : *rel_varpointsto) {
				int s1, s3;
				std::string s2, s4;
				std::cout << "arity of varpointsto: " << t.size() << "\n";
				t >> s1 >> s2 >> s3 >> s4;
				std::cout << s1 << "-" << s2 << "-" << s3 << "-" << s4 << "\n";
				break;
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
