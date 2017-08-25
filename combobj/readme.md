## Compile the analysis rules

souffle -c -o 1-object-sensitive 1-object-sensitive.dl

(1-object-sensitive.dl is the concatenated version.)

## Run original analysis

./1-object-sensitive -j[NumOfThreads] -F[OriginalFactDir] -D[OriginalDBDir]

## Relabel facts into combined version

g++ [name].cpp -std=c++11 -o [name]

[name] = [findeqv, applyreplace]

1. ./findeqv [analysis_result_file] [replace_file]

extract equivalent classes from original facts
// TODO: explain how to see equivalent classes
generate *replace.csv*, each line represents a replacement rule formatted as "[Element]\t[NewElement]", here [NewElement] is the md5 code of the "class feature" // TODO: Is class feature a good name?

2. ./applyreplace [original_facts_dir] [new_facts_dir] [replace_file]

apply the replacement rules to facts
generate "new" facts

## Run analysis on new facts

./1-object-sensitive -j[NumOfThreads] -F[NewFactDir] -D[NewDBDir]

## Experimental Discovery

* [Incorrectness] The result (Stats_Simple_InsensCallGraphEdge.csv) is not sound. The old version deduces more facts than the new one.

