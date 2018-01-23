#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <functional>
#include <string>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <unordered_set>
#include <unordered_map>
using namespace std;

const int linebufsize = 10000;
const int MAX_CONTEXT_LEN = 3;
const int MAX_FACT_LEN = (MAX_CONTEXT_LEN + 1) << 1;

hash<string> str_hash;

struct Fact {
	string s[MAX_FACT_LEN];
	Fact() {
		for (int i = 0; i < MAX_FACT_LEN; i++)
			s[i] = "";
	}
	string to_s() const {
		string res = s[0];
		for (int i = 1; i < MAX_FACT_LEN; i++)
			res += "&" + s[i];
		return res;
	}
	bool operator==(const Fact& rhs) const {
		for (int i = 0; i < MAX_FACT_LEN; i++)
			if (s[i] != rhs.s[i])
				return false;
		return true;
	}
};

namespace std {
	template <> struct hash<Fact> {
		size_t operator()(const Fact &x) const {
			return str_hash(x.to_s());
		}
	};
}

struct Context {
	string s[MAX_CONTEXT_LEN];
	string to_s() {
		string res = s[0];
		for (int i = 1; i < MAX_CONTEXT_LEN; i++)
			res += "&" + s[i];
		return res;
	}
};

Fact replaceWithPlaceholder(Fact fact, string st) {
	for (int i = 0; i < MAX_FACT_LEN; i++)
		if (fact.s[i] == st)
			fact.s[i] = "@PlcHd$"; // placeholder
	return fact;
}

unordered_map<string, size_t> mp;
unordered_set<size_t> hset;
unordered_multimap<size_t, string> h2s;
unordered_map<string, Context> id2c, id2hc;
int hclen, clen;
unordered_set<Fact> factset;

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
		++tot;
		if (!(tot & 0x1fffff)) printf("\t%dth element\n", tot);
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
			Context ctx;
			switch (i) {
				case 0:
				ctx = id2hc[sub];
				for (int j = 0; j < hclen; j++)
					fact.s[j] = ctx.s[j];
				break;
				case 1:
				fact.s[hclen] = sub;
				break;
				case 2:
				ctx = id2c[sub];
				for (int j = 0; j < clen; j++)
					fact.s[hclen+1+j] = ctx.s[j];
				break;
				case 3:
				fact.s[hclen+1+clen] = sub;
			}
			i++;
		} while (left < s.size());
		unordered_set<string> set_of_strings(fact.s, fact.s+hclen+clen+2);
		for (auto &st : set_of_strings) {
			if (st == "") continue;
			Fact rp_fact = replaceWithPlaceholder(fact, st);
			string rp_fact_st = rp_fact.to_s();
			size_t h = str_hash(rp_fact_st);
			add2mp(st, h);
		}
	}
	fclose(iFile);
	printf("\n#VarPoints = %d\n", tot);
}

int process_context_file(char* context_file, unordered_map<string, Context>& ctx_mapping) {
	static char linebuf[linebufsize];
	FILE* iFile = fopen(context_file, "r");
	int tot;
	int n;
	while (fgets(linebuf, linebufsize, iFile) != NULL) {
		string s(linebuf);
		++tot;
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
		n = i-1;
	}
	fclose(iFile);
	return n;
}

void process_database_files(char* varPointsTo_file, char* unfoldedHContext_file, char* unfoldedContext_file) {
	printf("processing UnfoldedHContext file: %s ...\n", unfoldedHContext_file);
	hclen = process_context_file(unfoldedHContext_file, id2hc);
	printf("processing UnfoldedContext file: %s ...\n", unfoldedContext_file);
	clen = process_context_file(unfoldedContext_file, id2c);
	printf("dim(hcxt) = %d, dim(cxt) = %d, |hctx| = %lu, |ctx| = %lu.\n", hclen, clen, id2hc.size(), id2c.size());
	printf("processing VarPointsTo file: %s ...\n", varPointsTo_file);
	process_varPointsTo_file(varPointsTo_file);
}

void generate_replace_file(char* replace_file) {
	for (auto it = mp.begin(); it != mp.end(); it++)
		hset.insert(it->second);
	for (auto it = mp.begin(); it != mp.end(); it++) {
		h2s.insert(make_pair(it->second, it->first));
	}
	//printf("Size of each equivalent class (>=2):\n");
	FILE* oFile = fopen(replace_file, "w");
	for (auto it = hset.begin(); it != hset.end(); it++) {
		if (h2s.count(*it) >= 2) { // !!!!!!!!!!!!!!!!!!!!!!!!!! >= 2
			printf("%lu ", h2s.count(*it));
			auto eqvcls = h2s.equal_range(*it);
			string representive = (eqvcls.first)->second.c_str();
			for (auto it2 = eqvcls.first; it2 != eqvcls.second; it2++) {
				//if (it2->second != representative)
				fprintf(oFile, "%s\t%s\n", it2->second.c_str(), representive.c_str()); // first element as representive
			}
		}
	}
	printf("\n#Elements = %lu, #Equivalent classes = %lu.\n", mp.size(), hset.size());
	fclose(oFile);
	/*
	auto eqvcls = h2s.equal_range(best_hash_value);
	for (auto it = eqvcls.first; it != eqvcls.second; it++)
		printf("%s\n", it->second.c_str());
	*/
}

int main(int argc, char* argv[]) { 
	// argv[1-4]: VarPointsTo, UnfoldedHContext_file, UnfoldedContext_file, replace_file
	process_database_files(argv[1], argv[2], argv[3]);
	generate_replace_file(argv[4]);
	return 0;
}
