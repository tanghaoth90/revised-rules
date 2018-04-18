import csv, sys
from collections import Counter

db_unfold = sys.argv[1] + "/"
#replace1 = sys.argv[2]

'''
vpt_set = set()
with open(db_unfold+"VarPointsTo.csv", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	for row in reader:
		vpt_set.add(tuple([int(x) for x in row]))
shif_set = set()
with open(db_unfold+"StoreHeapInstanceField.csv", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	for row in reader:
		shif_set.add(tuple(row))
ovmib_set = set()
with open(db_unfold+"OptVirtualMethodInvocationBase.csv", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	for row in reader:
		ovmib_set.add(tuple(row))
base_itsct = set([vpt[4] for vpt in vpt_set]) & set([ovmib[1] for ovmib in ovmib_set])
'''
cge_set = set()
with open(db_unfold+"CallGraphEdge.csv", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	for row in reader:
		cge_set.add(tuple([int(x) for x in row]))
tpt_set = set()
with open(db_unfold+"ThrowPointsTo.csv", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	for row in reader:
		tpt_set.add(tuple([int(x) for x in row]))
pts_itsct = set([cge[3:] for cge in cge_set]) & set([tpt[2:] for tpt in tpt_set])

'''
dc = {}
for vpt in vpt_set:
	#if vpt[4] not in base_itsct: continue
	key = (vpt[0], vpt[2], vpt[3])
	value = ('VarPointsTo', vpt[1], vpt[4])
	dc[key] = (dc[key] if key in dc else 0) ^ hash(value)
for shif in shif_set:
	if True: 
		key = shif[3:5]
		value = shif[3:] + ('StoreHeapInstanceField', shif[5])
		dc[key] = (dc[key] if key in dc else 0) ^ hash(value)
for ovmib in ovmib_set:
	if (ovmib[1] in base_itsct):
		key = ovmib[1]
		value = ovmib[:1] + ('OptVirtualMethodInvocationBase', )
		dc[key] = (dc[key] if key in dc else 0) ^ hash(value)
for cge in cge_set:
	if cge[3:] in pts_itsct: 
		key = cge[3:]
		value = ('CallGraphEdge', ) + cge[:3]
		dc[key] = (dc[key] if key in dc else 0) ^ hash(value)
		
hv2objs = {}
for k, hv in dc.items():
	if hv not in hv2objs:
		hv2objs[hv] = set([k])
	else:
		hv2objs[hv].add(k)
print "Multi-dimensional Compression"
print len(dc), len(hv2objs)
print Counter([len(objs) for hv, objs in hv2objs.items()])
for hv, objs in hv2objs.items():
	if (len(objs) == 20):
		print '\n' + '\n'.join([str(t) for t in sorted(list(objs))]) + '\n'
'''

dc = {}
for tpt in tpt_set:
	if tpt[2:] in pts_itsct: 
		key = tpt[2:]
		value = ('ThrowPointsTo', ) + tpt[:2]
		dc[key] = (dc[key] if key in dc else 0) ^ hash(value)

hv2objs = {}
for k, hv in dc.items():
	if hv not in hv2objs:
		hv2objs[hv] = set([k])
	else:
		hv2objs[hv].add(k)
print "Multi-dimensional Compression"
print len(dc), len(hv2objs)
print Counter([len(objs) for hv, objs in hv2objs.items()])
for hv, objs in hv2objs.items():
	#print len(objs), 
	if (len(objs) == 8):
		#print '\n' + '\n'.join([str(t) for t in sorted(list(objs))]) + '\n'
		print
		flag = True
		for o in objs:
			if flag: 
				print sorted([tpt[:2] for tpt in tpt_set if tpt[2:] == o])
				flag = False
			print o, '&', sorted([cge[:3] for cge in cge_set if cge[3:] == o])
		
