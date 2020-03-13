# subgraph-isomorphism

1.Make sure you will not insert two same edge (two edges have same source,target,label);
2.Node label should be size_t type. If your node label type is something else,you should turn it into size_t and range from 0,1,2,...n. It is not nessceary that  0,1,2 label exist if you have node with label 3.But do not like this example 0,1,1000000(it will cost numbers of memory).
3.Induce graph isomorphism algorithm now work well,but graph isomorphism algorithm not work.