#pragma once
size_t calHashSuitableSize(const size_t need) 
{
	size_t i = 8;
	while (i < need) i = i << 1;
	if (i * 0.8 > need) return i;
	else return i << 1;
};



