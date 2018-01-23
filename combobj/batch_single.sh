#subjects="antlr chart eclipse fop luindex lusearch pmd xalan" # bloat"
subjects="luindex lusearch xalan"
for s in $subjects; do
	make run_self_training analysis=$1 subject=$s;
done
