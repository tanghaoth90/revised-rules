#subject=fop
data_dir=/home/tanghao/data
subject_dir=$(data_dir)/$(subject)
ori_facts=$(subject_dir)/facts
ori_db=$(subject_dir)/ori_db
replace_file=$(data_dir)/replace_by_pts
new_facts=$(subject_dir)/newfacts
new_db=$(subject_dir)/new_db
analysis=2o1h
analysis_noout=$(analysis)_noout
threads=8
$(analysis): $(analysis).dl
	date
	souffle -c -o $(analysis) $(analysis).dl -p $(analysis).log >/dev/null 2>&1
	date
run: $(analysis)
	date
	if [ ! -d $(ori_db) ]; then mkdir $(ori_db); fi
	./$(analysis) -j$(threads) -F$(ori_facts) -D$(ori_db)
	date
$(analysis_noout): $(analysis_noout).dl
	date
	souffle -c -o $(analysis_noout) $(analysis_noout).dl -p $(analysis_noout).log >/dev/null 2>&1
	date
run_noout: $(analysis_noout) 
	date
	if [ ! -d $(new_db) ]; then mkdir $(new_db); fi
	./$(analysis_noout) -j$(threads) -F$(new_facts) -D$(new_db)
	date
findeqv: findeqv.cpp
	g++ -std=c++11 -o findeqv findeqv.cpp
run_findeqv: findeqv
	date 2>&1 | tee findeqv.log
	./findeqv $(ori_db)/VarPointsTo.csv $(ori_db)/UnfoldedHContext.csv $(ori_db)/UnfoldedContext.csv $(replace_file)_$(subject).csv 2>&1 | tee -a findeqv.log
	date 2>&1 | tee -a findeqv.log
findeqv_insens: findeqv.cpp
	g++ -std=c++11 -o findeqv_insens findeqv_insens.cpp
run_findeqv_insens: findeqv_insens
	date 2>&1 | tee findeqv.log
	./findeqv_insens $(ori_db)/VarPointsTo.csv $(ori_db)/UnfoldedHContext.csv $(ori_db)/UnfoldedContext.csv $(replace_file)_$(subject).csv 2>&1 | tee -a findeqv.log
	date 2>&1 | tee -a findeqv.log
findeqv_type: findeqv_type.cpp
	g++ -std=c++11 -o findeqv_type findeqv_type.cpp
run_findeqv_type: findeqv_type
	date 2>&1 | tee findeqv_type.log
	./findeqv_type $(ori_db)/Value_Type.csv $(replace_file) 2>&1 | tee -a findeqv_type.log
	date 2>&1 | tee -a findeqv_type.log
applyreplace: applyreplace.cpp
	g++ -std=c++11 -o applyreplace applyreplace.cpp
run_applyreplace: applyreplace
	if [ ! -d $(new_facts) ]; then mkdir $(new_facts); fi
	#./applyreplace $(ori_facts) $(new_facts) $(replace_file)_sum.csv
	./applyreplace $(ori_facts) $(new_facts) $(replace_file)_$(subject).csv
composite: composite.cpp
	g++ -std=c++11 -o composite composite.cpp
run_composite: composite
	./composite $(replace_file)_sum.csv $(replace_file)_eclipse.csv $(replace_file)_lusearch.csv # $(replace_file)_antlr.csv $(replace_file)_chart.csv
ls_uf:
	wc -l $(ori_db)/UnfoldedHContext.csv
	wc -l $(ori_db)/UnfoldedContext.csv
ls_old_db:
	wc -l $(ori_db)/*.csv
ls_new_db:
	wc -l $(new_db)/*.csv
clean_ori_db:
	rm -r $(ori_db)
clean_analysis:
	rm $(analysis) $(analysis).log $(analysis).cpp *.ccerr
clean_findeqv:
	rm findeqv findeqv.log	
clean_applyreplace:
	rm applyreplace
clean_all: clean_ori_db clean_analysis clean_findeqv
