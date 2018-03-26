import csv, sys
from collections import Counter
import itertools

db_unfold = sys.argv[1] + "/"
#replace1 = sys.argv[2]

def extract_by_indices(t, indices):
	return tuple([t[i] for i in indices])

def load_rel_from_file(rel_file):
	rel_set = set()
	with open(db_unfold+rel_file, "rb") as csvfile:
		reader = csv.reader(csvfile, delimiter='\t')
		for row in reader:
			fact = tuple([int(x) for x in row])
			rel_set.add(fact)
	return rel_set

def load_rel_by_key(rel_set, k_indices, v_indices):
	rel_dict = {}
	for fact in rel_set:
		key = extract_by_indices(fact, k_indices)
		value = extract_by_indices(fact, v_indices)
		if key not in rel_dict: rel_dict[key] = []
		rel_dict[key].append(value)
	return rel_dict
			
def cal_eqv(rel_set, k_indices, v_indices):
	key2hash = {}
	for fact in rel_set:
		key = extract_by_indices(fact, k_indices)
		value = extract_by_indices(fact, v_indices)
		key2hash[key] = (key2hash[key] if key in key2hash else 0) ^ hash(value)
	print len(key2hash.keys())
	hash_values = key2hash.values()
	hash_counter = Counter(hash_values)
	hash_values = set(hash_values)
	print len(hash_values)
	rep = {}
	notrep = set()
	for key in key2hash:
		if key2hash[key] not in rep: 
			rep[key2hash[key]] = key
		else:
			notrep.add(key)
	print len(rel_set)
	print len([fact for fact in rel_set if extract_by_indices(fact, k_indices) not in notrep])

cge_set = load_rel_from_file('CallGraphEdge.csv')
tpt_set = load_rel_from_file('ThrowPointsTo.csv')

cge_dict = load_rel_by_key(cge_set, range(3,6), range(0,3))
tpt_dict = load_rel_by_key(tpt_set, range(2,5), range(0,2))

insth_list = [y+x for k in cge_dict if k in tpt_dict for x in cge_dict[k] for y in tpt_dict[k]]
multi_count = Counter(insth_list).values()
print len(insth_list) #, sum(multi_count)
print Counter(multi_count)

cal_eqv(set(insth_list), [0,2,3], [1,4])

'''
storehif_sss = load_rel_from_file('StoreHeapInstanceField.csv')
vpt_sss = load_rel_from_file('VarPointsTo.csv')

storehif_set = load_rel_by_key(storehif_sss, range(3,6), range(0,3))
vpt_set = load_rel_by_key(vpt_sss, range(2,5), range(0,2))

iftp_set =	[x+y for k in storehif_set if k in vpt_set for x in storehif_set[k] for y in vpt_set[k]]
print iftp_set[0]
multi_count = Counter(iftp_set).values()
print sum(multi_count)
freq_count = Counter(multi_count)
print freq_count
print sum(freq_count.values())
'''
