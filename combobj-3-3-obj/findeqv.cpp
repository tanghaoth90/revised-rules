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

const int linebufsize = 10000;
hash<string> str_hash;

struct Fact {
	string s[8];
	string to_string() {
		int i;
		string res = s[0];
		for (i = 1; i < 8; i++)
			res += "&" + s[i];
		return res;
	}
};

struct Context {
	string s[3];
	string to_string() {
		int i;
		string res = s[0];
		for (i = 1; i < 3; i++)
			res += "&" + s[i];
		return res;
	}
};

Fact replaceWithPlaceholder(Fact fact, string st) {
	int i;
	for (i = 0; i < 8; i++)
		if (fact.s[i] == st)
			fact.s[i] = "<*Placeholder*>";
	return fact;
}

unordered_map<string, size_t> mp;
unordered_set<size_t> hset;
unordered_multimap<size_t, string> h2s;
unordered_map<string, Context> id2c, id2hc;

void add2mp(string key, size_t new_hash) {
	mp[key] = mp[key] ^ new_hash;
}

void process_varPointsTo_file(char* varPointsTo_file) {
	static char linebuf[linebufsize];
	int tot = 0;
	string fname(varPointsTo_file);
	string file_relation = fname.substr(0, fname.size()-4).c_str();
	FILE* iFile = fopen(varPointsTo_file, "r");
	while (fgets(linebuf, linebufsize, iFile) != NULL) {
		string s(linebuf);
		if (++tot % 100000 == 0) printf("%d\n", tot);
		size_t left = 0, sublen;
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
		unordered_set<string> set_of_strings(fact.s, fact.s+8);
		//if (tot < 5) printf("%s\n", fact.to_string().c_str());
		for (auto &st : set_of_strings) {
			Fact rp_fact = replaceWithPlaceholder(fact, st);
			string rp_fact_st = rp_fact.to_string();
			size_t h = str_hash(rp_fact_st);
			add2mp(st, h);
			//if (tot < 5) printf("  %s\n", rp_fact_st.c_str());
		}
	}
	fclose(iFile);
	printf("#VarPoints = %d\n", tot);
}

void process_context_file(char* context_file, unordered_map<string, Context>& ctx_mapping) {
	static char linebuf[linebufsize];
	FILE* iFile = fopen(context_file, "r");
	int tot;
	while (fgets(linebuf, linebufsize, iFile) != NULL) {
		string s(linebuf);
		if (++tot % 100000 == 0) printf("%d\n", tot);
		size_t left = 0, sublen;
		string id;
		int i = 0;
		do {
			sublen = s.substr(left).find("\t");
			if (sublen == -1) sublen = s.size() - left;
			string sub = s.substr(left, sublen);
			boost::trim_right(sub);	
			boost::trim_left(sub);
			left += 1 + sublen;
			if (i == 0)
				id = sub;			
			else
				ctx_mapping[id].s[i-1] = sub;
			i++;
		} while (left < s.size());
	}
	fclose(iFile);
}

void process_database_files(char* varPointsTo_file, char* unfoldedHContext_file, char* unfoldedContext_file) {
	process_context_file(unfoldedHContext_file, id2hc);
	printf("id2hc.size() = %lu\n", id2hc.size());
	printf("1 -> %s\n", id2hc["1"].to_string().c_str());
	printf("2 -> %s\n", id2hc["2"].to_string().c_str());
	process_context_file(unfoldedContext_file, id2c);
	printf("id2c.size() = %lu\n", id2c.size());
	printf("3 -> %s\n", id2c["3"].to_string().c_str());
	printf("4 -> %s\n", id2c["4"].to_string().c_str());
	process_varPointsTo_file(varPointsTo_file);
}

int main(int argc, char* argv[]) {
	process_database_files(argv[1], argv[2], argv[3]);

	for (auto it = mp.begin(); it != mp.end(); it++)
		hset.insert(it->second);
	for (auto it = mp.begin(); it != mp.end(); it++) {
		h2s.insert(make_pair(it->second, it->first));
	}
	printf("Size of each equivalent class (>=2):\n");
	bool flag = false;
	char* replace_file = argv[4];
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
