subject=antlr
subject_dir=/home/tanghao/software/doop/out/context-insensitive/souffle-$(subject)
ori_facts=$(subject_dir)/facts
ori_db=$(subject_dir)/ori_db
replace_file=$(ori_db)/replace.csv
analysis=3o3h
compile_analysis:
	date
	souffle -c -o $(analysis) $(analysis).dl -p $(analysis).log >/dev/null 2>&1
	date
run_ori: compile_analysis 
	date
	if [ ! -d $(ori_db) ]; then mkdir $(ori_db); fi
	./$(analysis) -j8 -F$(ori_facts) -D$(ori_db)
	wc -l $(ori_db)/*.csv
	date
compile_findeqv:
	g++ -std=c++11 -o findeqv findeqv.cpp
run_findeqv: compile_findeqv
	./findeqv $(ori_db)/VarPointsTo.csv $(ori_db)/UnfoldedHContext.csv $(ori_db)/UnfoldedContext.csv $(replace_file)
ls_uf:
	wc -l $(ori_db)/UnfoldedHContext.csv
	wc -l $(ori_db)/UnfoldedContext.csv
clean:
	rm -r $(ori_db)
