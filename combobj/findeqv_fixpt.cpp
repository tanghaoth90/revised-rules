#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <functional>
#include <algorithm>
#include <string>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <unordered_set>
#include <unordered_map>
using namespace std;

const int linebufsize = 10000;
const int MAX_CONTEXT_LEN = 3;
const int MAX_FACT_LEN = (MAX_CONTEXT_LEN + 1) << 1;

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

hash<string> str_hash;
namespace std {
	template <> struct hash<Fact> {
		size_t operator()(const Fact &x) const {
			return str_hash(x.to_s());
		}
	};
}
hash<Fact> fact_hash;

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
unordered_set<Fact> fact_set;

inline void add2mp(string key, size_t new_hash) {
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
		if (!(tot & 0x7ffff)) printf("\treading %dth fact\n", tot);
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
		fact_set.insert(fact);
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

unordered_map<string,string> parent; // disjoint sets
unordered_map<string,int> size_of_set;

inline bool is_represent(string element) {
	return parent[element] == element;
}

string findset(string element) {
	if (is_represent(element))
		return element;
	else
		return parent[element] = findset(parent[element]);
}

// @return representative element for element1 & element2
string combine(string element1, string element2) {
	element1 = findset(element1);
	element2 = findset(element2);
	size_of_set[element1] += size_of_set[element2];
	return parent[element2] = element1;
}

void init_disjoint_sets() {
	printf("initializing disjoint sets ...\n");
	for (auto &fact : fact_set) {
		for (int i = 0; i < hclen+clen+2; i++) {
			parent[fact.s[i]] = fact.s[i];
			size_of_set[fact.s[i]] = 1;
		}
	}
	printf(" DONE~\n");
}

// todo: string -> size_t for hash(string)
void replace_once() {
	int dealing = 0;
	for (auto it = fact_set.begin(); it != fact_set.end(); ) {
		Fact fact = *it;
		bool changed = false;
		for (int i = 0; i < hclen+clen+2; i++)
			if (!is_represent(fact.s[i])) {
				fact.s[i] = findset(fact.s[i]);
				changed = true;
			}
		dealing++;
		if (!(dealing & 0x7ffff))
			printf("\tdealing %dth fact\n", dealing);
		if (changed) {
			it = fact_set.erase(it);
			fact_set.insert(fact);
		}
		else
			it++;
	}
	printf("%lu\n", fact_set.size());
	mp.clear();
	dealing = 0;
	for (auto &fact : fact_set) {
		string sset[8];
		auto sset_last = copy(fact.s, fact.s+hclen+clen+2, sset);
		sort(sset, sset_last);
		sset_last = unique(sset, sset_last);
		for (auto sset_it = sset; sset_it != sset_last; sset_it++) {
			string st = *sset_it;
			Fact rp_fact = replaceWithPlaceholder(fact, st);
			add2mp(st, fact_hash(rp_fact));
		}
		dealing++;
		if (!(dealing & 0x7ffff))
			printf("\tdealing %dth fact\n", dealing);
	}
	hset.clear();
	h2s.clear();
	printf("%lu\n", fact_set.size());
	for (auto it = mp.begin(); it != mp.end(); it++)
		hset.insert(it->second);
	for (auto it = mp.begin(); it != mp.end(); it++) {
		h2s.insert(make_pair(it->second, it->first));
	}
	int flag = 0;
	for (auto it = hset.begin(); it != hset.end(); it++) {
		if (h2s.count(*it) >= 2) { 
			flag++;
			auto eqvcls = h2s.equal_range(*it);
			string representative = (eqvcls.first)->second.c_str();
			for (auto it2 = eqvcls.first; it2 != eqvcls.second; it2++) {
				if (it2->second != representative)
					combine(representative, it2->second);
			}
		}
	}
	printf("combine time = %d\n", flag);
}

void generate_replace_file(char* replace_file) {
	init_disjoint_sets();
	replace_once();
	//replace_once();
	//replace_once();
	//printf("Size of each equivalent class (>=2):\n");
	FILE* oFile = fopen(replace_file, "w");
	size_t num_of_sets = 0;
	for (auto it = parent.begin(); it != parent.end(); it++) {
		if (size_of_set[findset(it->first)] >= 2)
			fprintf(oFile, "%s\t%s\n", it->first.c_str(), findset(it->first).c_str()); 
		if (is_represent(it->first))
			num_of_sets ++;
	}
	fclose(oFile);
	printf("\n#Elements = %lu, #Equivalent classes = %lu.\n", parent.size(), num_of_sets);
}

int main(int argc, char* argv[]) { 
	// argv[1-4]: VarPointsTo, UnfoldedHContext_file, UnfoldedContext_file, replace_file
	process_database_files(argv[1], argv[2], argv[3]);
	generate_replace_file(argv[4]);
	return 0;
}
