subjects="fop" # bloat"
#subjects="antlr chart eclipse fop luindex lusearch pmd xalan" # bloat"
for s in $subjects; do
	make run_self_training analysis=$1 subject=$s;
done
