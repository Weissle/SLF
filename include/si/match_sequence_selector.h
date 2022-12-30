#pragma once
#include "predefined.h"
#include "tools/fenwick.h"
#include <map>
#include <queue>
#include <tuple>
#include <vector>
namespace slf
{
class match_sequence_selector
{

public:
    match_sequence_selector(const graph_t& _query, const graph_t& _target)
        : query(_query), target(_target)
    {
    }
    struct node_cmp_info
    {
        node_id_t id;
        size_t indeg, outdeg;
        size_t match_node_number;
        node_cmp_info(node_id_t _id, size_t _indeg, size_t _outdeg)
            : id(_id), indeg(_indeg), outdeg(_outdeg), match_node_number(0)
        {
        }
    };
    struct priority_info
    {
        node_id_t id;
        size_t connect;
        int neg_match_node_number;
        size_t degree;
        priority_info() = default;
        priority_info(node_id_t _id, size_t _connect, int _match_node_number,
                      size_t _degree)
            : id(_id), connect(_connect), neg_match_node_number(-_match_node_number),
              degree(_degree)
        {
        }
        bool operator<(const priority_info& r) const
        {
            return std::tie(connect, neg_match_node_number, degree) <
                   std::tie(r.connect, r.neg_match_node_number, r.degree);
        }
    };
    std::vector<node_id_t> run();

private:
    std::vector<int> get_query_node_match_number();
    void prepare_possibility_info();
    void count_possible_match_node(std::vector<node_cmp_info>& q,
                                   std::vector<node_cmp_info>& t);
    std::vector<node_id_t>
    get_match_sequence(const std::vector<int>& match_node_num);
    std::map<node_label_t, std::vector<node_cmp_info>> query_mp, target_mp;
    std::priority_queue<priority_info> pq;
    fenwick<int> fenwick_;
    const graph_t &query, &target;
};

} // namespace slf
