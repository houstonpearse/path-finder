function test () {
    a=$(mktemp)
    ./path-finder < tests/test$1.txt > a
    if diff a tests/test$1-out.txt; then 
        echo "✅ Test $1 Passed"
    else
        echo "❌ Test $1 Failed"
    fi
    rm a
}
for i in $(seq 0 $1)
do
    test $i
done