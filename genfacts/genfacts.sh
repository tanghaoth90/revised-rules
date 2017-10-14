#!/bin/bash
# Usage: sh genfacts.sh 2>&1 | tee genfacts.log
# cd ~/software/doop
for subject in antlr bloat chart eclipse fop luindex lusearch pmd xalan; do
	echo "************************"
	echo "Running ${subject}"
	echo "************************"
	./doop -a context-insensitive -i jars/dacapo/${subject}.jar --dacapo --id souffle-${subject} --souffle --souffle-jobs 8
done
