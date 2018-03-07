subjects="antlr chart eclipse fop luindex lusearch pmd xalan" # bloat"
for s in $subjects; do
	make run_use_sum subject=$s analysis=$1;
done
