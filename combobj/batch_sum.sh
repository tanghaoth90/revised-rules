#subjects="antlr chart eclipse fop luindex lusearch pmd xalan" # bloat"
subjects="antlr eclipse fop luindex lusearch xalan" # bloat"
for s in $subjects; do
	make run_use_sum subject=$s;
done
