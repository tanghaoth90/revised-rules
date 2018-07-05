import sys
import relutil
import csv
from collections import Counter

def sumkey(t):
	return hash(t) & 0xfff

if __name__ == "__main__":
	db_unfold = sys.argv[1] + "/"
	rel_file_loader = lambda rel: relutil.load_rel_from_file(db_unfold+rel+'.csv')
	runmode = int(sys.argv[2])
	if runmode == 0:
		cge_set = rel_file_loader('CallGraphEdge')
		tpt_set = rel_file_loader('ThrowPointsTo')
		key_set = set()
		for cge in cge_set:
			key_set.add(cge[2:])
		print len(key_set)
		x_set = set()
		for tpt in tpt_set:
			x_set.add(tpt[2:])
		print len(x_set)
	elif runmode == 1:
		cge_set = rel_file_loader('CallGraphEdge')
		tpt_set = rel_file_loader('ThrowPointsTo')
		cge_dict = relutil.index_rel(cge_set, range(3,6), range(0,3))
		tpt_dict = relutil.index_rel(tpt_set, range(2,5), range(0,2))
		insth_full_list = [y[:2]+x for k in cge_dict if k in tpt_dict for x in cge_dict[k] for y in tpt_dict[k]]
		insth_list = [t[:5] for t in insth_full_list]
		insth_set = set(insth_list)
		insth_dict = relutil.index_rel(insth_full_list, [4], [0,1,2,3,4,5,6,7])
		multi_count = Counter(insth_list).values()
		print '|INSTH list| = ', len(insth_list) #, sum(multi_count)
		print '|INSTH set| = ', len(insth_set)
		print Counter(multi_count)
		#relutil.cal_eqv_loop(insth_set, 5)
		insth_eqv_class = relutil.cal_eqv(insth_set, [4], [0,1,2,3])[0]
		print len(insth_eqv_class)
		#tptformat = lambda y: y[:2]+y[5:]
		#cgeformat = lambda y: y[2:]
		#insthformat = lambda y: y[:5]
		tptformat = lambda y: ( sumkey(y[:2]), sumkey(y[5:]) )
		cgeformat = lambda y: ( sumkey(y[2:4]), sumkey(y[4]), sumkey(y[5:]) )
		insthformat = lambda y: ( sumkey(y[:2]), sumkey(y[2:4]), sumkey(y[4]) )
		i = 0
		for k in insth_eqv_class:
			if i == 10: break
			clz = insth_eqv_class[k]
			if len(clz) == 6:
				i += 1
				print "\n", clz
				for x in clz:
					#print "KEY (*, *, *, *, %d)" % x[0]
					print "KEY (*, *, " + str(sumkey(x[0])) + ")"
					pre = 0
					for y in sorted(insth_dict[x]):
						if pre != insthformat(y):
							print "I%s :- T%s, C%s" % ( str(insthformat(y)), str(tptformat(y)), str(cgeformat(y)) )
							pre = insthformat(y)
						else:
							print " %s :- T%s, C%s" % ( ' '*len(str(insthformat(y))), str(tptformat(y)), str(cgeformat(y)) )
	elif runmode == 2:	
		storehif_set = rel_file_loader('StoreHeapInstanceField')
		print 'SHIF'
		relutil.cal_eqv(storehif_set, [1,3,4], [0,2,5])
		vpt_set = rel_file_loader('VarPointsTo')
		print 'VPT'
		relutil.cal_eqv(vpt_set, [0,2,3], [1,4])
		storehif_dict = relutil.index_rel(storehif_set, range(3,6), range(0,3))
		vpt_dict = relutil.index_rel(vpt_set, range(2,5), range(0,2))
		iftp_list =	[y[:2]+x[:3] for k in storehif_dict if k in vpt_dict for x in storehif_dict[k] for y in vpt_dict[k]]
		iftp_set = set(iftp_list)
		multi_count = Counter(iftp_list).values()
		print '|IFTP list| = ', len(iftp_list), '; ', #, sum(multi_count)
		print '|IFTP set| = ', len(iftp_set)
		print Counter(multi_count)
		relutil.cal_eqv_loop(iftp_set, 5)
	elif runmode == 3:
		ov_set = rel_file_loader('OptVirtualMethodInvocationBase')
		relutil.cal_eqv_loop(ov_set, 2)
	elif runmode == 4:
		cge_set = rel_file_loader('CallGraphEdge')
		'''
		print 'CGE rp2'
		rp2 = relutil.cal_eqv(cge_set, [2], [0,1,3,4,5])[1]
		print 'CGE rp03'
		rp03 = relutil.cal_eqv(cge_set, [0,3], [1,2,4,5])[1]
		print 'CGE rp14'
		rp14 = relutil.cal_eqv(cge_set, [1,4], [0,2,3,5])[1]
		compress_l = len([cge for cge in cge_set if (((cge[0],cge[3]) in rp03) and ((cge[1],cge[4]) in rp14) and ((cge[2],) in rp2))])
		print 'CGE rate'
		relutil.print_rate(compress_l, len(cge_set))
		'''
		print 'CGE rp2345'
		(cge_ecmap, rp2345) = relutil.cal_eqv(cge_set, [2,3,4,5], [0,1])
		compress_l = len([cge for cge in cge_set if (cge[2],cge[3],cge[4],cge[5]) in rp2345])
		print 'CGE rate'
		relutil.print_rate(compress_l, len(cge_set))
		i2s = {}
		with open(db_unfold+"index2symbol.csv", "rb") as indexfile:
			reader = csv.reader(indexfile, delimiter='\t')
			for row in reader:
				i2s[int(row[0])] = row[1]
		with open("replace_cge.csv", "wb") as csvfile:
			writer = csv.writer(csvfile, delimiter='\t')
			for k in cge_ecmap:
				v = list(cge_ecmap[k])
				for vi in v:
					vv = map(lambda x: i2s[x], vi+v[0])
					writer.writerow(vv)
	elif runmode == 5:
		vpt_set = rel_file_loader('VarPointsTo')
		print 'VPT'
		cl02 = relutil.cal_eqv(vpt_set, [0,2], [1,3,4])[0]
		dict02 = relutil.index_rel(vpt_set, [0,2], [1,3,4])
		i = 0
		for e in cl02:
			if i > 10: break
			print '***',
			for d in cl02[e]:
				print sorted(dict02[d])

		rp02 = relutil.cal_eqv(vpt_set, [0,2], [1,3,4])[1]
		rp13 = relutil.cal_eqv(vpt_set, [1,3], [0,2,4])[1]
		rp4 = relutil.cal_eqv(vpt_set, [4], [0,1,2,3])[1]
		compress_l = len([vpt for vpt in vpt_set if (((vpt[0],vpt[2]) in rp02) and ((vpt[1],vpt[3]) in rp13) and ((vpt[4],) in rp4))])
		relutil.print_rate(compress_l, len(vpt_set))
	elif runmode == 6:
		tpt_set = rel_file_loader('ThrowPointsTo')
		#relutil.cal_eqv_loop(tpt_set, 5)
		tpt_ecmap = relutil.cal_eqv(tpt_set, [2,3,4], [0,1])[0]
		i2s = {}
		with open(db_unfold+"index2symbol.csv", "rb") as indexfile:
			reader = csv.reader(indexfile, delimiter='\t')
			for row in reader:
				i2s[int(row[0])] = row[1]
		with open("replace_tpt.csv", "wb") as csvfile:
			writer = csv.writer(csvfile, delimiter='\t')
			for k in tpt_ecmap:
				v = list(tpt_ecmap[k])
				for vi in v:
					vv = map(lambda x: i2s[x], vi+v[0])
					writer.writerow(vv)
	elif runmode == 7:
		valueType_set = rel_file_loader('Value_Type')
		valueType_map = {}
		for fact in valueType_set:
			if fact[0] in valueType_map: print '[Error] One instance maps to multiple types.'
			valueType_map[fact[0]] = fact[1]
		print len(valueType_set)
		instanceFieldPt_set = rel_file_loader('InstanceFieldPointsTo')
		print len(instanceFieldPt_set)
		instanceField2Type_set = set([(valueType_map[t[3]] if t[3] in valueType_map else -1,)+t[4:] for t in instanceFieldPt_set])
		print len(instanceField2Type_set)
		'''
		with open("1.csv", "wb") as csvfile:
			writer = csv.writer(csvfile, delimiter='\t')
			for t in instanceField2Type_set:
				writer.writerow(t)
		'''
		dict02 = relutil.index_rel(instanceField2Type_set, [2,3,4,5], [0,1])
		ft = {}
		hasdiff = set()
		for k in dict02:
			v = dict02[k]
			if k[3] in ft:

			ft[k[3]] = 