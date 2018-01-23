benchmark_dir=/home/tanghao/data
subject_dir=$(benchmark_dir)/$(subject)
facts=$(subject_dir)/facts
facts_opt=$(subject_dir)/facts_opt
db=$(subject_dir)/db
db_opt=$(subject_dir)/db_opt
replace_file=$(subject_dir)/replace_by_pts.csv
#analysis=2o1h
analysis_opt=$(analysis)_opt
threads=8
CC=g++ -std=c++11
results=results.log
replace_sum_file=$(benchmark_dir)/replace_sum.csv

$(analysis): $(analysis).dl
	souffle -c -o $(analysis) $(analysis).dl -p $(analysis).log #>/dev/null 2>&1
run_$(analysis): $(analysis)
	if [ ! -d $(db) ]; then mkdir $(db); fi
	./$(analysis) -j$(threads) -F$(facts) -D$(db)
$(analysis_opt): $(analysis_opt).dl
	souffle -c -o $(analysis_opt) $(analysis_opt).dl -p $(analysis_opt).log >/dev/null 2>&1
run_$(analysis_opt): $(analysis_opt) 
	if [ ! -d $(db_opt) ]; then mkdir $(db_opt); fi
	./$(analysis_opt) -j$(threads) -F$(facts_opt) -D$(db_opt)
	
findeqv: findeqv.cpp
	$(CC) -o findeqv findeqv.cpp
run_findeqv: findeqv
	./findeqv $(db)/VarPointsTo.csv $(db)/UnfoldedHContext.csv $(db)/UnfoldedContext.csv $(replace_file)
findeqv_fixpt: findeqv_fixpt.cpp
	$(CC) -o findeqv_fixpt findeqv_fixpt.cpp
run_findeqv_fixpt: findeqv_fixpt
	./findeqv_fixpt $(db)/VarPointsTo.csv $(db)/UnfoldedHContext.csv $(db)/UnfoldedContext.csv $(replace_file)
findeqv_insens: findeqv_insens.cpp
	$(CC) -o findeqv_insens findeqv_insens.cpp
run_findeqv_insens: findeqv_insens
	./findeqv_insens $(db)/VarPointsTo.csv $(db)/UnfoldedHContext.csv $(db)/UnfoldedContext.csv $(replace_file)
findeqv_type: findeqv_type.cpp
	$(CC) -o findeqv_type findeqv_type.cpp
run_findeqv_type: findeqv_type
	./findeqv_type $(db)/Value_Type.csv $(replace_file)

applyreplace: applyreplace.cpp
	$(CC) -o applyreplace applyreplace.cpp
run_applyreplace_sum: applyreplace
	if [ ! -d $(facts_opt) ]; then mkdir $(facts_opt); fi
	./applyreplace $(facts) $(facts_opt) $(replace_sum_file)
run_applyreplace_single: applyreplace
	if [ ! -d $(facts_opt) ]; then mkdir $(facts_opt); fi
	./applyreplace $(facts) $(facts_opt) $(replace_file)

run_self_training: run_$(analysis) run_findeqv_fixpt run_applyreplace_single run_$(analysis_opt)
	echo "[$(subject)]" >> $(results)
	echo "run_$(analysis)" >> $(results)
	souffle-profile $(analysis).log -c top >> $(results) 2>&1
	echo "run_$(analysis_opt)" >> $(results)
	souffle-profile $(analysis_opt).log -c top >> $(results) 2>&1
	echo "" >> $(results)

run_use_sum: run_composite run_applyreplace_sum run_$(analysis_opt)
	echo "[$(subject) S]" >> $(results)
	echo "run_$(analysis_opt)" >> $(results)
	souffle-profile $(analysis_opt).log -c top >> $(results) 2>&1
	echo "" >> $(results)

composite: composite.cpp
	$(CC) -o composite composite.cpp
run_composite: composite
	./composite $(replace_sum_file) $(subject) $(benchmark_dir)/antlr/replace_by_pts.csv $(benchmark_dir)/chart/replace_by_pts.csv $(benchmark_dir)/eclipse/replace_by_pts.csv $(benchmark_dir)/fop/replace_by_pts.csv $(benchmark_dir)/luindex/replace_by_pts.csv $(benchmark_dir)/lusearch/replace_by_pts.csv $(benchmark_dir)/pmd/replace_by_pts.csv $(benchmark_dir)/xalan/replace_by_pts.csv

ls_db:
	wc -l $(db)/*.csv
ls_db_opt:
	wc -l $(db_opt)/*.csv
clean_db:
	rm -r $(db)
clean_db_opt:
	rm -r $(db_opt)
clean_analysis:
	rm $(analysis) $(analysis).log $(analysis).cpp *.ccerr
clean_findeqv:
	rm findeqv findeqv_insens findeqv_type
clean_applyreplace:
	rm applyreplace
clean_all: clean_db clean_analysis clean_findeqv
