subject=antlr
subject_dir=/home/tanghao/software/doop/out/context-insensitive/souffle-$(subject)
ori_facts=$(subject_dir)/facts
ori_db=$(subject_dir)/ori_db
analysis=3o3h
compile:
	date
	souffle -c -o $(analysis) $(analysis).dl -p $(analysis).log >/dev/null 2>&1
	date
run_ori: compile
	date
	if [ ! -d $(ori_db) ]; then mkdir $(ori_db); fi
	./$(analysis) -j8 -F$(ori_facts) -D$(ori_db)
	wc -l $(ori_db)/*.csw
	date
clean:
	rm -r $(ori_db)
