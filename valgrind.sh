# check memory
#valgrind --leak-check=yes ./build/slf -c data/config.json

# callgrind
# valgrind -q --tool=callgrind ./build/slf -c data/config.json

# cache
valgrind -q --tool=cachegrind ./build/slf -c data/config.json

