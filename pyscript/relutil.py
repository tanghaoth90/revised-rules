import csv
from collections import Counter
import itertools

def extract_by_indices(t, indices):
	return tuple([t[i] for i in indices])

def load_rel_from_file(rel_file_name):
	rel_set = set()
	with open(rel_file_name, "rb") as csvfile:
		reader = csv.reader(csvfile, delimiter='\t')
		for row in reader:
			fact = tuple([int(x) for x in row])
			rel_set.add(fact)
	return rel_set

def index_rel(rel_set, k_indices, v_indices):
	rel_dict = {}
	for fact in rel_set:
		key = extract_by_indices(fact, k_indices)
		#value = extract_by_indices(fact, v_indices)
		if key not in rel_dict: rel_dict[key] = []
		rel_dict[key].append(fact)
	return rel_dict

def print_rate(part, whole):
	print "%d/%d (%.2f)" % (part, whole, part/float(whole))

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
	eqvclass = {}
	for key in key2hash:
		h = key2hash[key]
		if h not in rep: 
			rep[h] = key
			eqvclass[h] = set([key])
		else:
			notrep.add(key)
			eqvclass[h].add(key)
	reduced_facts = [fact for fact in rel_set if extract_by_indices(fact, k_indices) not in notrep]
	print_rate(len(reduced_facts), len(rel_set))
	#return eqvclass, set(rep.values())
	return set(rep.values())

def cal_eqv_loop(rel_set, arity):
	for S in range(2**arity):
		if S == 0 or S == (2**arity)-1: continue
		k_indices = filter(lambda x: ((1<<x)&S)!=0, range(arity))
		v_indices = filter(lambda x: ((1<<x)&S)==0, range(arity))
		print k_indices, 
		cal_eqv(rel_set, k_indices, v_indices)
		
