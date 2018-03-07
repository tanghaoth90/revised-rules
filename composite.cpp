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
		for (i = 1; i < 3; i++)
			res += "&" + s[i];
		return res;
	}
};

Fact replaceWithPlaceholder(Fact fact, string st) {
	if (fact.s[1] == st)
		fact.s[1] = "<*Placeholder*>";
	return fact;
}

unordered_map<string, size_t> mp;
unordered_set<size_t> hset;
unordered_multimap<size_t, string> h2s;
int hclen, clen;

void add2mp(string key, size_t new_hash) {
	mp[key] = mp[key] ^ new_hash;
}

void process_replace_file(char* replace_file) {
	static char linebuf[linebufsize];
	int tot = 0;
	string fname(replace_file);
	string file_relation = fname.substr(0, fname.size()-4).c_str();
	FILE* iFile = fopen(replace_file, "r");
	while (fgets(linebuf, linebufsize, iFile) != NULL) {
		string s(linebuf);
		if (++tot % 500000 == 0) printf("%d\n", tot);
		size_t left = 0, sublen;
		Fact fact;
		int i = 0;
		fact.s[i++] = file_relation;
		do {
			sublen = s.substr(left).find("\t");
			if (sublen == -1) sublen = s.size() - left;
			string sub = s.substr(left, sublen);
			boost::trim_right(sub);	
			boost::trim_left(sub);
			left += 1 + sublen;
			fact.s[i] = sub;
			i++;
		} while (left < s.size());
		unordered_set<string> set_of_strings(fact.s+1, fact.s+2);
		//if (tot == 1) printf("%s\n", fact.to_string().c_str());
		for (auto &st : set_of_strings) {
			if (st == "") continue;
			Fact rp_fact = replaceWithPlaceholder(fact, st);
			string rp_fact_st = rp_fact.to_string();
			size_t h = str_hash(rp_fact_st);
			add2mp(st, h);
			//if (tot == 1) printf("  %s\n", rp_fact_st.c_str());
		}
	}
	fclose(iFile);
	printf("%s #rp = %d\n", replace_file, tot);
}

void generate_summary_replace_file(char* replace_file) {
	for (auto it = mp.begin(); it != mp.end(); it++)
		hset.insert(it->second);
	for (auto it = mp.begin(); it != mp.end(); it++) {
		h2s.insert(make_pair(it->second, it->first));
	}
	printf("Size of each equivalent class (>=2):\n");
	//bool flag = false;
	FILE* oFile = fopen(replace_file, "w");
	for (auto it = hset.begin(); it != hset.end(); it++) {
		if (h2s.count(*it) >= 2) {
			auto eqvcls = h2s.equal_range(*it);
			printf("%lu ", h2s.count(*it));
			/*
			if (!flag && h2s.count(*it) >= 10 && h2s.count(*it) < 15) {
				flag = true;
				auto eqvcls = h2s.equal_range(*it);
				for (auto it2 = eqvcls.first; it2 != eqvcls.second; it2++)
					printf("%s\n", it2->second.c_str());
			}
			*/
			string representive = (eqvcls.first)->second.c_str();
			for (auto it2 = eqvcls.first; it2 != eqvcls.second; it2++) {
				//fprintf(oFile, "%s\t%lx\n", it2->second.c_str(), it2->first); // md5 as representive
				if (it2 != eqvcls.first) {
					fprintf(oFile, "%s\t%s\n", it2->second.c_str(), representive.c_str()); // first element as representive
				}
			}
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
}

int main(int argc, char* argv[]) { 
	// argv[1-*]: replace_sum file, replace_by_pts_* file
	puts("1");
	for (int j = 3; j < argc; j++) {
		printf("%lu\n", string(argv[j]).find(string(argv[2])));
		if (string(argv[j]).find(string(argv[2])) == -1) {
			printf("%s\n", argv[j]);
			process_replace_file(argv[j]);
		}
	}
	puts("2");
	generate_summary_replace_file(argv[1]);
	return 0;
}
