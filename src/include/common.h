#pragma once
size_t calHashSuitableSize(const size_t need) 
{
	size_t i = 8;
	while (i < need) i = i << 1;
	if (i * 0.8 > need) return i;
	else return i << 1;
};

#define PRINT_TIME_COST_S(INFO,T) std::cout<< INFO<< double(T)/CLOCKS_PER_SEC<<endl
#define PRINT_TIME_COST_MS(INFO,T) std::cout<< INFO<< double(T)/(CLOCKS_PER_SEC/1000)<<endl
#define LOOP(V,H,T) for(int V = H ;V<T;++V)


