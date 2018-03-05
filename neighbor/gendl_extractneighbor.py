import os, sys
fact_dir = sys.argv[1]
with open("extract_neighbor.dl", "w") as dl_script:
	dl_script.write(".type String\n")
	dl_script.write(".decl Center(c:String)\n")
	dl_script.write("Center(\"<The Center>\").\n")
	for filename in filter(lambda x:os.path.splitext(x)[1]=='.facts', os.listdir(fact_dir)):
		relname = os.path.splitext(filename)[0]
		relname = relname.replace('-','')
		with open(fact_dir+"/"+filename, "r") as fact_file:
			arity = len(fact_file.readline().split("\t"))
#			print "%s %d" % (relname, arity)
			arg_defs = reduce((lambda a,b: a+", "+b), map(lambda x:("p%d:String"%x), range(arity)))
			dl_script.write(".decl %s(%s)\n" % (relname, arg_defs))
			dl_script.write(".decl %s_Nbr(%s)\n" % (relname, arg_defs))
			dl_script.write(".input %s(IO=\"file\", filename=\"%s\", delimiter=\"\\t\")\n" % (relname, filename))
			for ec in range(arity):
				args = reduce((lambda a,b: a+","+b), map(lambda x:("p%d"%x) if x!=ec else "c", range(arity)))
				dl_script.write("%s_Nbr(%s) :- Center(c), %s(%s).\n" % (relname, args, relname, args))
			dl_script.write(".output %s_Nbr(IO=stdout)\n" % relname)
