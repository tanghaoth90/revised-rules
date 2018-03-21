import csv, sys

db_unfold = sys.argv[1] + "/"
#replace1 = sys.argv[2]

vpt_set = set()
with open(db_unfold+"VarPointsTo.csv", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	for row in reader:
		vpt_set.add(tuple(row))
'''
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
cge_set = set()
with open(db_unfold+"CallGraphEdge.csv", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	for row in reader:
		cge_set.add(tuple(row))
tpt_set = set()
with open(db_unfold+"ThrowPointsTo.csv", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	for row in reader:
		tpt_set.add(tuple(row))
pts_itsct = set([cge[3:] for cge in cge_set]) & set([tpt[2:] for tpt in tpt_set])
'''

dc = {}
for vpt in vpt_set:
	#if vpt[4] not in base_itsct: continue
	key = vpt[4]
	value = vpt[:4] + ('VarPointsTo', )
	dc[key] = (dc[key] if key in dc else 0) ^ hash(value)
'''
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
for tpt in tpt_set:
	if tpt[2:] in pts_itsct: 
		key = tpt[2:]
		value = ('ThrowPointsTo', ) + tpt[:2]
		dc[key] = (dc[key] if key in dc else 0) ^ hash(value)
'''

hv2rep = {}
count = {}
for k, hv in dc.items():
	if hv not in hv2rep:
		hv2rep[hv] = k
		count[hv] = 1
	else:
		count[hv] += 1
print "Multi-dimensional Compression"
print len(dc), len(hv2rep)

