# subgraph-isomorphism
A parallel algorithms for subgraph isomorphism or subgraph monomorphism.

## :warning:Warning  ##
* Edges with same source, target and label will only leave one.  
* Node label should be size_t type. If your node label type is something else,you should turn it into size_t and range from 0,1,2,...n.  
* Subgraph isomorphism algorithm now work well, but subgraph monomorphism algorithm may not work (not test yet).  
* Be careful when you use fstream::eof() to check if fstream has reached the end of file, it may have bug if there is a space in the end of file.  
```
fstream f; 
//open file
while(f.eof()==false){
	int i;
	cin>>i;
	cout<<i;
}
// file contain  : 1 2 3 4 5  (A space at the end of file),it will output 123455.
```

## How To Use ##

### Build ###
```sh
git clone https://github.com/Weissle/SLF.git && cd SLF && mkdir  build && cd build 
cmake ../src/ && make
```

### Use ###
```sh
# num is how many threads you want to use.
./slf ../test/pattern.graph ../test/target.graph -t {num}
# output format [{query_graph},{target_graph},{solutions_count},{time_usage}]
# [pattern.graph,target.graph,48,0.001212]
```
