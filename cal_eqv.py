import csv

dc = {}
with open("CallGraphEdge.log", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	for row in reader:
		key = tuple(row[3:])
		pre_hashval = dc[key] if key in dc else 0
		dc[key] = pre_hashval ^ hash(tuple(row[:2])) # may need to add relation name here
with open("ThrowPointsTo.log", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	for row in reader:
		key = tuple(row[2:])
		pre_hashval = dc[key] if key in dc else 0
		dc[key] = pre_hashval ^ hash(tuple(row[:1])) # may need to add relation name here
hv2rep = {}
notrep = set()
for k, hv in dc.items():
	if hv not in hv2rep:
		hv2rep[hv] = k
	else:
		notrep.add(k)
print "Multi-dimensional Compression"
print len(dc)
print len(hv2rep)
print len(notrep)
rate = 1
with open("CallGraphEdge.log", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	counter, counter0 = 0, 0
	for row in reader:
		key = tuple(row[3:])
		if key not in notrep:
			counter += 1
		counter0 += 1
	rate *= counter0 / float(counter)
	print "CallGraphEdge %d out of %d (%.2lf%%)" % (counter, counter0, counter*100.0/counter0)
with open("ThrowPointsTo.log", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	counter, counter0 = 0, 0
	for row in reader:
		key = tuple(row[2:])
		if key not in notrep:
			counter += 1
		counter0 += 1
	rate *= counter0 / float(counter)
	print "ThrowPointsTo %d out of %d (%.2lf%%)" % (counter, counter0, counter*100.0/counter0)
print "rate=%.2lf" % rate

dc0, dc1, dc2 = {}, {}, {}
with open("CallGraphEdge.log", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	for row in reader:
		key0 = row[3]
		pre_hashval = dc0[key0] if key0 in dc0 else 0
		dc0[key0] = pre_hashval ^ hash(tuple(row[:2]+row[4:])) # may need to add relation name here
		key1 = row[4]
		pre_hashval = dc1[key1] if key1 in dc1 else 0
		dc1[key1] = pre_hashval ^ hash(tuple(row[:3]+row[5:])) # may need to add relation name here
		key2 = row[5]
		pre_hashval = dc2[key2] if key2 in dc2 else 0
		dc2[key2] = pre_hashval ^ hash(tuple(row[:4]+row[6:])) # may need to add relation name here
with open("ThrowPointsTo.log", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	for row in reader:
		key0 = row[2]
		pre_hashval = dc0[key0] if key0 in dc0 else 0
		dc0[key0] = pre_hashval ^ hash(tuple(row[:1]+row[3:])) # may need to add relation name here
		key1 = row[3]
		pre_hashval = dc1[key1] if key1 in dc1 else 0
		dc1[key1] = pre_hashval ^ hash(tuple(row[:2]+row[4:])) # may need to add relation name here
		key2 = row[4]
		pre_hashval = dc2[key2] if key2 in dc2 else 0
		dc2[key2] = pre_hashval ^ hash(tuple(row[:3]+row[5:])) # may need to add relation name here
hv2rep0, hv2rep1, hv2rep2 = {}, {}, {}
notrep0, notrep1, notrep2 = set(), set(), set()
for k, hv in dc0.items():
	if hv not in hv2rep0:
		hv2rep0[hv] = k
	else:
		notrep0.add(k)
for k, hv in dc1.items():
	if hv not in hv2rep1:
		hv2rep1[hv] = k
	else:
		notrep1.add(k)
for k, hv in dc2.items():
	if hv not in hv2rep2:
		hv2rep2[hv] = k
	else:
		notrep2.add(k)
print "Single Dimensional Compression"
print len(dc0), len(dc1), len(dc2)
print len(hv2rep0), len(hv2rep1), len(hv2rep2)
print len(notrep0), len(notrep1), len(notrep2)
rate = 1
with open("CallGraphEdge.log", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	counter, counter0 = 0, 0
	for row in reader:
		if row[3] not in notrep0 and row[4] not in notrep1 and row[5] not in notrep2:
			counter += 1
		counter0 += 1
	rate *= counter0 / float(counter)
	print "CallGraphEdge %d out of %d (%.2lf%%)" % (counter, counter0, counter*100.0/counter0)
with open("ThrowPointsTo.log", "rb") as csvfile:
	reader = csv.reader(csvfile, delimiter='\t')
	counter, counter0 = 0, 0
	for row in reader:
		key = tuple(row[2:])
		if row[2] not in notrep0 and row[3] not in notrep1 and row[4] not in notrep2:
			counter += 1
		counter0 += 1
	rate *= counter0 / float(counter)
	print "ThrowPointsTo %d out of %d (%.2lf%%)" % (counter, counter0, counter*100.0/counter0)
print "rate=%.2lf" % rate

	
