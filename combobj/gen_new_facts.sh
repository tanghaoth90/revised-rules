g++ findeqv.cpp -std=c++11 -o findeqv
g++ applyreplace.cpp -std=c++11 -o applyreplace
./findeqv 1-obj/database/VarPointsTo.csv replace.csv
./applyreplace subfacts newfacts replace.csv
