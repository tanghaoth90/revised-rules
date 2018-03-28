import csv, sys
from collections import Counter
import itertools

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
		#value = extract_by_indices(fact, v_indices)
		if key not in rel_dict: rel_dict[key] = []
		rel_dict[key].append(fact)
	return rel_dict
			
def cal_eqv(rel_set, k_indices, v_indices):
	key2hash = {}
	for fact in rel_set:
		key = extract_by_indices(fact, k_indices)
		value = extract_by_indices(fact, v_indices)
		key2hash[key] = (key2hash[key] if key in key2hash else 0) ^ hash(value)
	#print len(key2hash.keys())
	hash_values = key2hash.values()
	hash_counter = Counter(hash_values)
	hash_values = set(hash_values)
	#print len(hash_values)
	rep = {}
	notrep = set()
	for key in key2hash:
		if key2hash[key] not in rep: 
			rep[key2hash[key]] = key
		else:
			notrep.add(key)
	reduced_facts = [fact for fact in rel_set if extract_by_indices(fact, k_indices) not in notrep]
	return len(reduced_facts), len(rel_set), float(len(reduced_facts)) / len(rel_set)

if __name__ == "__main__":
	db_unfold = sys.argv[1] + "/"
	#replace1 = sys.argv[2]
	runmode = sys.argv[2]
	if runmode == '1':
		cge_set = load_rel_from_file('CallGraphEdge.csv')
		print 'CGE', cal_eqv(cge_set, [0,1,3,4], [2,5])
		tpt_set = load_rel_from_file('ThrowPointsTo.csv')
		print 'TPT', cal_eqv(tpt_set, [0,2,3], [1,4])
		cge_dict = load_rel_by_key(cge_set, range(3,6), range(0,3))
		tpt_dict = load_rel_by_key(tpt_set, range(2,5), range(0,2))
		insth_list = [y[:2]+x[:3] for k in cge_dict if k in tpt_dict for x in cge_dict[k] for y in tpt_dict[k]]
		insth_set = set(insth_list)
		multi_count = Counter(insth_list).values()
		print 'INSTH list len = ', len(insth_list) #, sum(multi_count)
		print 'INSTH set len = ', len(insth_set)
		print Counter(multi_count)
		print cal_eqv(insth_set, [0,2,3], [1,4])
	elif runmode == '2':	
		storehif_set = load_rel_from_file('StoreHeapInstanceField.csv')
		print 'SHIF', cal_eqv(storehif_set, [1,3,4], [0,2,5])
		vpt_set = load_rel_from_file('VarPointsTo.csv')
		print 'VPT', cal_eqv(vpt_set, [0,2,3], [1,4])
		storehif_dict = load_rel_by_key(storehif_set, range(3,6), range(0,3))
		vpt_dict = load_rel_by_key(vpt_set, range(2,5), range(0,2))
		iftp_list =	[y[:2]+x[:3] for k in storehif_dict if k in vpt_dict for x in storehif_dict[k] for y in vpt_dict[k]]
		iftp_set = set(iftp_list)
		multi_count = Counter(iftp_list).values()
		print 'IFTP list len = ', len(iftp_list) #, sum(multi_count)
		print 'IFTP set len = ', len(iftp_set)
		print Counter(multi_count)
		print cal_eqv(iftp_set, [0,3], [1,2,4])
		print cal_eqv(iftp_set, [3,4], [0,1,2])
	
