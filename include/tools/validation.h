#include "predefined.h"
#include <map>
#include <set>
#include <vector>
namespace slf
{

inline bool induce_check(const graph_t& q, const graph_t& t,
                         std::vector<node_id_t> qms, std::vector<node_id_t> tms)
{
    std::set<node_id_t> set_q, set_t;
    std::map<node_id_t, node_id_t> map_q, map_t;
    assert(qms.size() == tms.size());
    for (size_t i = 0; i < qms.size(); i++)
    {
        set_q.insert(qms[i]);
        set_t.insert(tms[i]);
        map_q[qms[i]] = tms[i];
        map_t[tms[i]] = qms[i];
    }
    auto check = [](const graph_t& g1, const graph_t& g2,
                    std::set<node_id_t> s1, std::map<node_id_t, node_id_t>& mp)
    {
        for (auto id : s1)
        {
            auto& node = g1.get_node(id);
            assert(mp.count(id));
            if (node.label() != g2.get_node(mp[id]).label())
                return false;
            if (node.get_self_loop() != g2.get_node(mp[id]).get_self_loop())
                return false;
            for (auto e : node.source_edges())
            {
                auto source = e.source();
                if (s1.count(source) == 0)
                    continue;
                if (g2.has_edge(mp[source], mp[id], e.label()) == false)
                    return false;
            }
            for (auto e : node.target_edges())
            {
                auto target = e.target();
                if (s1.count(target) == 0)
                    continue;
                if (g2.has_edge(mp[id], mp[target], e.label()) == false)
                    return false;
            }
        }
        return true;
    };
    return check(q, t, set_q, map_q) && check(t, q, set_t, map_t);
}

} // namespace slf
