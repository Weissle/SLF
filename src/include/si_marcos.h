#define getMapValue_C(M,K) ( M . find( K ) == M . end()) ? (decltype(M)::value_type::second_type() ):( M . find( K )->second);



#define getMapValue(M,K) M[K]



