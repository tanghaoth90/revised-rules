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
unordered_map<string, string> replacePattern;
const string facts_sfx(".facts");

inline bool ends_with(string const & value, string const & ending) {
	if (ending.size() > value.size()) return false;
	return equal(ending.rbegin(), ending.rend(), value.rbegin());
}

int main(int argc, char* argv[]) {
	char* ori_facts_dir_name = argv[1];
	char* new_facts_dir_name = argv[2];
	char* replace_file = argv[3];
	string ori_facts_dir(ori_facts_dir_name);
	string new_facts_dir(new_facts_dir_name);

	string fname(replace_file);
	int tot = 0;
	FILE* iFile = fopen(fname.c_str(), "r");
	while (fgets(linebuf, linebufsize, iFile) != NULL) {
		string s(linebuf);
		if (++tot % 500000 == 0) printf("%d\n", tot);
		size_t left = 0, sublen;
		string outline;
		string rp[2];
		int i = 0;
		do {
			sublen = s.substr(left).find("\t");
			if (sublen == -1) sublen = s.size() - left;
			string sub = s.substr(left, sublen);
			boost::trim_right(sub);	
			boost::trim_left(sub);
			left += 1 + sublen;
			rp[i++] = sub;
		} while (left < s.size());
		replacePattern[rp[0]] = rp[1];
	}
	fclose(iFile);
	printf("#ReplacedElement = %d\n", tot);

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(ori_facts_dir.c_str())) == NULL) return 1;
	tot = 0;
	while ((ent = readdir(dir)) != NULL) {
		string fname(ent->d_name);
		if (!ends_with(fname, facts_sfx)) continue;
		printf("%s\n", ent->d_name);
		iFile = fopen((ori_facts_dir+"/"+fname).c_str(), "r");
		FILE* oFile = fopen((new_facts_dir+"/"+fname).c_str(), "w");
		while (fgets(linebuf, linebufsize, iFile) != NULL) {
			string s(linebuf);
			if (++tot % 100000 == 0) printf("%d\n", tot);
			size_t left = 0, sublen;
			//if (tot < 5)
			//	printf("A %s", s.c_str());
			string outline;
			do {
				sublen = s.substr(left).find("\t");
				if (sublen == -1) sublen = s.size() - left;
				string sub = s.substr(left, sublen);
				boost::trim_right(sub);	
				boost::trim_left(sub);
				if (replacePattern.count(sub)) sub = replacePattern[sub];
				outline += (outline.size() ? "\t" : "") + sub;
				left += 1 + sublen;
			} while (left < s.size());
			//if (tot < 5)
			//	printf("O %s\n", outline.c_str());
			fputs((outline+"\n").c_str(), oFile);
		}
		fclose(iFile);
		fclose(oFile);
	}

	return 0;
}
