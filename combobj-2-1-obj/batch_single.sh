subjects="antlr bloat chart eclipse fop luindex lusearch pmd xalan"
for s in $subjects; do
	make run_self_training subject=$s;
done
