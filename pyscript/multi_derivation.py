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
	return len(reduced_facts), len(rel_set), float(len(reduced_facts)) / len(rel_set), eqvclass, set(rep.values())

def cal_eqv_loop(rel_set, arity):
	for S in range(2**arity):
		if S == 0 or S == (2**arity)-1: continue
		k_indices = filter(lambda x: ((1<<x)&S)!=0, range(arity))
		v_indices = filter(lambda x: ((1<<x)&S)==0, range(arity))
		print k_indices, v_indices, cal_eqv(rel_set, k_indices, v_indices)[:3]

if __name__ == "__main__":
	db_unfold = sys.argv[1] + "/"
	#replace1 = sys.argv[2]
	runmode = sys.argv[2]
	if runmode == '1':
		cge_set = load_rel_from_file('CallGraphEdge.csv')
		#cge_e = cal_eqv(cge_set, [1,2,3,4,5], [0])
		cge_e = cal_eqv(cge_set, [2,3,4,5], [0,1])
		print 'CGE', cge_e[:3]
		cge_e = cal_eqv(cge_set, [2,5], [0,1,3,4])
		print 'CGE', cge_e[:3]
		cge_e = cal_eqv(cge_set, [2], [0,1,3,4,5])
		print 'CGE', cge_e[:3]
		cge_e = cal_eqv(cge_set, [3,4,5], [0,1,2])
		print 'CGE', cge_e[:3]
		tpt_set = load_rel_from_file('ThrowPointsTo.csv')
		tpt_e = cal_eqv(tpt_set, [2,3,4], [0,1])
		print 'TPT', tpt_e[:3]
		cge_dict = load_rel_by_key(cge_set, range(3,6), range(0,3))
		tpt_dict = load_rel_by_key(tpt_set, range(2,5), range(0,2))
		insth_list = [y[:2]+x[:3] for k in cge_dict if k in tpt_dict for x in cge_dict[k] for y in tpt_dict[k]]
		insth_set = set(insth_list)
		multi_count = Counter(insth_list).values()
		print 'INSTH list len = ', len(insth_list) #, sum(multi_count)
		print 'INSTH set len = ', len(insth_set)
		print Counter(multi_count)
		#cal_eqv_loop(insth_set, 5)
		print cal_eqv(insth_set, [4], [0,1,2,3])
	elif runmode == '2':	
		storehif_set = load_rel_from_file('StoreHeapInstanceField.csv')
		storehif_e = cal_eqv(storehif_set, [1,3,4], [0,2,5])
		print 'SHIF', storehif_e[:3]
		vpt_set = load_rel_from_file('VarPointsTo.csv')
		vpt_e = cal_eqv(vpt_set, [0,2,3], [1,4])
		print 'VPT', vpt_e[:3]
		storehif_dict = load_rel_by_key(storehif_set, range(3,6), range(0,3))
		vpt_dict = load_rel_by_key(vpt_set, range(2,5), range(0,2))
		iftp_list =	[y[:2]+x[:3] for k in storehif_dict if k in vpt_dict for x in storehif_dict[k] for y in vpt_dict[k]]
		iftp_set = set(iftp_list)
		multi_count = Counter(iftp_list).values()
		print 'IFTP list len = ', len(iftp_list) #, sum(multi_count)
		print 'IFTP set len = ', len(iftp_set)
		print Counter(multi_count)
		#print cal_eqv(iftp_set, [0,3], [1,2,4])
		#print cal_eqv(iftp_set, [3,4], [0,1,2])
		cal_eqv_loop(iftp_set, 5)
	elif runmode == '3':
		ov_set = load_rel_from_file('OptVirtualMethodInvocationBase.csv')
		cal_eqv_loop(ov_set, 2)
	elif runmode == '4':
		cge_set = load_rel_from_file('CallGraphEdge.csv')
		#cal_eqv_loop(cge_set, 6)
		print cal_eqv(cge_set, [2], [0,1,3,4,5])[:3]
		print cal_eqv(cge_set, [0,3], [1,2,4,5])[:3]
		print cal_eqv(cge_set, [1,4], [0,2,3,5])[:3]
		rp2 = cal_eqv(cge_set, [2], [0,1,3,4,5])[4]
		rp03 = cal_eqv(cge_set, [0,3], [1,2,4,5])[4]
		rp14 = cal_eqv(cge_set, [1,4], [0,2,3,5])[4]
		compress_l = len([cge for cge in cge_set if (((cge[0],cge[3]) in rp03) and ((cge[1],cge[4]) in rp14) and ((cge[2],) in rp2))])
		print (compress_l, len(cge_set), float(compress_l) / len(cge_set))
	elif runmode == '5':
		vpt_set = load_rel_from_file('VarPointsTo.csv')
		#cal_eqv_loop(vpt_set, 5)
		print cal_eqv(vpt_set, [0,2], [1,3,4])[:3]
		print cal_eqv(vpt_set, [1,3], [0,2,4])[:3]
		print cal_eqv(vpt_set, [4], [0,1,2,3])[:3]
		rp02 = cal_eqv(vpt_set, [0,2], [1,3,4])[4]
		rp13 = cal_eqv(vpt_set, [1,3], [0,2,4])[4]
		rp4 = cal_eqv(vpt_set, [4], [0,1,2,3])[4]
		compress_l = len([vpt for vpt in vpt_set if (((vpt[0],vpt[2]) in rp02) and ((vpt[1],vpt[3]) in rp13) and ((vpt[4],) in rp4))])
		print (compress_l, len(vpt_set), float(compress_l) / len(vpt_set))
	elif runmode == '6':
		tpt_set = load_rel_from_file('ThrowPointsTo.csv')
		cal_eqv_loop(tpt_set, 5)
	
