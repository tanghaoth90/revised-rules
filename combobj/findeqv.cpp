#include <fstream>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <unordered_map>
#include <dirent.h>
#include <vector>
#include <unordered_set>
using namespace std;

const int linebufsize = 20000;
char linebuf[linebufsize];
hash<string> str_hash;

struct Fact {
	string s[4];
	string to_string() {
		return s[0] + "#" + s[1] + "#" + s[2] + "#" + s[3];
		//return s[0].substr(0,15) + "#" + s[1].substr(0,15) + "#" + s[2].substr(0,15) + "#" + s[3].substr(0,15);
	}
};

Fact replaceWithUniq(Fact fact, string st) {
	int i;
	for (i = 0; i < 4; i++)
		if (fact.s[i] == st)
			fact.s[i] = "<<uniq>>";
	return fact;
}

unordered_map<string, size_t> mp;
unordered_set<size_t> hset;
unordered_multimap<size_t, string> h2s;

void add2mp(string key, size_t new_hash) {
	mp[key] = mp[key] ^ new_hash;
}

int main(int argc, char* argv[]) {
	char* analysis_result_file = argv[1];
	char* replace_file = argv[2];
	int tot = 0;
	//string fname("1-obj/database/VarPointsTo.csv");
	string fname(analysis_result_file);
	string file_relation = fname.substr(0, fname.size()-4).c_str();
	FILE* iFile = fopen(fname.c_str(), "r");
	while (fgets(linebuf, linebufsize, iFile) != NULL) {
		string s(linebuf);
		if (++tot % 500000 == 0) printf("%d\n", tot);
		size_t left = 0, sublen;
		string outline;
		Fact fact;
		int i = 0;
		do {
			sublen = s.substr(left).find("\t");
			if (sublen == -1) sublen = s.size() - left;
			string sub = s.substr(left, sublen);
			boost::trim_right(sub);	
			boost::trim_left(sub);
			left += 1 + sublen;
			fact.s[i++] = sub;
		} while (left < s.size());
		unordered_set<string> set_of_strings(fact.s, fact.s+4);
		//if (tot < 5) printf("%s\n", fact.to_string().c_str());
		for (auto &st : set_of_strings) {
			Fact rp_fact = replaceWithUniq(fact, st);
			string rp_fact_st = rp_fact.to_string();
			size_t h = str_hash(rp_fact_st);
			add2mp(st, h);
			//if (tot < 5) printf("  %s\n", rp_fact_st.c_str());
		}
	}
	fclose(iFile);
	printf("#VarPoints = %d\n", tot);

	for (auto it = mp.begin(); it != mp.end(); it++)
		hset.insert(it->second);
	for (auto it = mp.begin(); it != mp.end(); it++) {
		h2s.insert(make_pair(it->second, it->first));
	}
	printf("Size of each equivalent class (>=2):\n");
	bool flag = false;

	//FILE* oFile = fopen("replace.csv", "w");
	FILE* oFile = fopen(replace_file, "w");
	for (auto it = hset.begin(); it != hset.end(); it++) {
		if (h2s.count(*it) > 2) {
			printf("%lu ", h2s.count(*it));
			/*
			if (!flag && h2s.count(*it) >= 10 && h2s.count(*it) < 15) {
				flag = true;
				auto eqvcls = h2s.equal_range(*it);
				for (auto it2 = eqvcls.first; it2 != eqvcls.second; it2++)
					printf("%s\n", it2->second.c_str());
			}
			*/
			auto eqvcls = h2s.equal_range(*it);
			for (auto it2 = eqvcls.first; it2 != eqvcls.second; it2++)
				fprintf(oFile, "%s\t%lx\n", it2->second.c_str(), it2->first);
		}
	}
	printf("\n#Elements = %lu\n", mp.size());
	printf("#Equivalent classes = %lu\n", hset.size());
	fclose(oFile);
	/*
	auto eqvcls = h2s.equal_range(best_hash_value);
	for (auto it = eqvcls.first; it != eqvcls.second; it++)
		printf("%s\n", it->second.c_str());
	*/
	return 0;
}
