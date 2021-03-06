benchmark_dir=/home/tanghao/data
subject_dir=$(benchmark_dir)/$(subject)
facts=$(subject_dir)/facts
#facts_opt=$(subject_dir)/facts_opt
facts_opt=$(facts)
cfg=$(benchmark_dir)/cfg
db=$(subject_dir)/db
db_opt=$(subject_dir)/db_opt
db_unfold=$(subject_dir)/db_unfold
replace1=$(facts)/replace1.csv
analysis_opt=$(analysis)_opt
threads=8
results=results.log
replace_sum_file=$(benchmark_dir)/replace_sum.csv

ctxlen=3
hctxlen=3
analysis=pts$(ctxlen)o$(hctxlen)h
analysis_interface=$(analysis)_interface
analysis_unfold=$(analysis)_unfold

CC=g++ -std=c++11

$(analysis): $(analysis).dl
	souffle -c -o $(analysis) $(analysis).dl -p $(analysis).log >/dev/null 2>&1
run_$(analysis): $(analysis)
	if [ ! -d $(db) ]; then mkdir $(db); fi
	./$(analysis) -j$(threads) -F$(facts) -D$(db) #-p $(analysis).log
$(analysis)_mahjong: $(analysis)_mahjong.dl
	souffle -c -o $(analysis)_mahjong $(analysis)_mahjong.dl -p $(analysis)_mahjong.log >/dev/null 2>&1
run_$(analysis)_mahjong: $(analysis)_mahjong
	if [ ! -d $(db) ]; then mkdir $(db); fi
	./$(analysis)_mahjong -j$(threads) -F$(facts) -D$(db) #-p $(analysis).log
$(analysis)_itsc: $(analysis)_itsc.dl
	souffle -c -o $(analysis)_itsc $(analysis)_itsc.dl -p $(analysis)_itsc.log >/dev/null 2>&1
run_$(analysis)_itsc: $(analysis)_itsc
	if [ ! -d $(db) ]; then mkdir $(db); fi
	./$(analysis)_itsc -j$(threads) -F$(facts) -D$(db) #-p $(analysis).log
$(analysis_opt): $(analysis_opt).dl
	souffle -c -o $(analysis_opt) $(analysis_opt).dl -p $(analysis_opt).log >/dev/null 2>&1
run_$(analysis_opt): $(analysis_opt) 
	if [ ! -d $(db_opt) ]; then mkdir $(db_opt); fi
	./$(analysis_opt) -j$(threads) -F$(facts_opt) -D$(db_opt) #-p $(analysis_opt).log
$(analysis_interface).cpp: $(analysis).dl
	souffle -g $(analysis_interface).cpp -j$(threads) $(analysis).dl >/dev/null 2>&1
$(analysis_unfold): unfold_results.cpp $(analysis_interface).cpp
	$(CC) -I/usr/include/souffle -fopenmp -DUSE_PROVENANCE -O3 -DUSE_LIBZ -DUSE_SQLITE -D__EMBEDDED_SOUFFLE__ -o $(analysis_unfold) unfold_results.cpp $(analysis_interface).cpp -lpthread -lsqlite3 -lz -lncurses -D CTX_LEN=$(ctxlen) -D HCTX_LEN=$(hctxlen) >/dev/null 2>&1
run_$(analysis_unfold): $(analysis_unfold)
	if [ ! -d $(db_unfold) ]; then mkdir $(db_unfold); fi
	./$(analysis_unfold) $(analysis_interface) $(facts) $(db_unfold) $(cfg)/rel.schema
run_cal_eqv:
	python cal_eqv.py $(db_unfold) $(replace1)

run_self_training: run_$(analysis) run_findeqv_fixpt run_applyreplace_single run_$(analysis_opt)
	echo "[$(subject)]" >> $(results)
	echo "run_$(analysis)" >> $(results)
	souffle-profile $(analysis).log -c top >> $(results) 2>&1
	echo "run_$(analysis_opt)" >> $(results)
	souffle-profile $(analysis_opt).log -c top >> $(results) 2>&1
	echo "" >> $(results)
	
run_self_training_inspect: run_$(analysis) run_$(analysis)_inspect run_applyreplace_single run_$(analysis_opt)
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
clean_all: clean_db clean_analysis
