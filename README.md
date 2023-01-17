# SLF
SLF is a parallel algorithms for subgraph isomorphism.  
**Paper name**: SLF: A passive parallelization of subgraph isomorphism  
**Paper link**: https://www.sciencedirect.com/science/article/abs/pii/S0020025522015286  

## :warning:Warning  ##
* If you want to re-produce the experiment in the paper, please checkout `master` branch and see the `README.md` at that branch.
* The same edges (same source, target and label) will only leave one.
* This version cannot solve subgraph monomorphism problem. (WIP, need dataset to test.)

## How To Use ##

### Dependency
A compiler supports `C++17` is required.  
`boost` is required.
```sh
# In ubuntu, you can use this command to install boost.
sudo apt install libboost-all-dev # We get boost-1.71, but other versions should work.
```
`google/benchmark` is optional and you don't need it if you don't know what it is.

### Build ###
```sh
git clone https://github.com/Weissle/SLF.git && cd SLF 
mkdir build && cd build 
cmake -DCMAKE_BUILD_TYPE=RELEASE ../ 
# build
make -j
# test
make test
# run example
cd ..
./build/slf -c data/config.json
# cat slf.log and see the result
cat slf.log
```

### Config ###
```
// Copy from the data/config.json
{
    "log":{
        "path":"slf.log", // Log path. Log to console if the path is empty.
        "level":"debug"   // Log level. Use "info" to ignore unimportant log.
    },
    "slf":{
        "thread_number":2,
        "graph_format":"grf",
        "max_log_results":1,                 // At most log {max_log_results} mappings. 0 means unlimited.
        "search_results_limitation": 0,      // Stop searching when find {search_results_limitation} results. 0 means unlimited.
        "search_time_limitation_seconds": 0, // Stop searching when a task takes more than {search_time_limitation_seconds} seconds. 0 means unlimited.
        // The task we want to run.
        "tasks":[         
            {
                "query": "data/query.graph",
                "target": "data/target.graph"
            },
            {
                "query": "data/si2_r001_m400.02.query.graph",
                "target": "data/si2_r001_m400.02.target.graph"
            }            
        ]
    }
}

```

