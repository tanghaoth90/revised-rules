import os
fact_dir=r'test'
for filename in filter(lambda x:os.path.splitext(x)[1]=='.facts', os.listdir(fact_dir)):
	print filename
	relname = os.path.splitext(filename)[0]
	print relname
	
