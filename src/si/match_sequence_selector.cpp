#include "si/match_sequence_selector.h"
#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <unordered_set>
namespace slf
{
void match_sequence_selector::prepare_possibility_info()
{
    size_t query_node_number = query.node_number();
    size_t target_node_number = target.node_number();
    for (size_t i = 0; i < query_node_number; i++)
    {
        auto& node = query.get_node(i);
        query_mp[node.label()].emplace_back(i, node.in_degree(),
                                            node.out_degree());
    }
    for (size_t i = 0; i < target_node_number; i++)
    {
        auto& node = target.get_node(i);
        target_mp[node.label()].emplace_back(i, node.in_degree(),
                                             node.out_degree());
    }
}

void match_sequence_selector::count_possible_match_node(
    std::vector<node_cmp_info>& q, std::vector<node_cmp_info>& t)
{
    if (q.size() > t.size())
        return;
    auto cmp_f = [](const node_cmp_info& l, const node_cmp_info& r)
    { return l.indeg > r.indeg; };
    sort(q.begin(), q.end(), cmp_f);
    sort(t.begin(), t.end(), cmp_f);
    auto max_deg =
        std::max_element(q.begin(), q.end(),
                         [](const node_cmp_info& l, const node_cmp_info& r)
                         { return l.outdeg < r.outdeg; })
            ->outdeg;
    fenwick_.reset(max_deg, 1);
    size_t t_idx = 0, t_num = t.size();
    for (auto& info : q)
    {
        while (t_idx < t_num && t[t_idx].indeg >= info.indeg)
        {
            auto outdeg = t[t_idx].outdeg;
            fenwick_.add((outdeg >= max_deg) ? max_deg : outdeg, 1);
            ++t_idx;
        }
        info.match_node_number = t_idx - fenwick_.query(info.outdeg - 1);
    }
}

std::vector<int> match_sequence_selector::get_query_node_match_number()
{
    size_t query_node_number = query.node_number();
    for (auto& kv : query_mp)
    {
        count_possible_match_node(kv.second, target_mp[kv.first]);
    }
    std::vector<bool> is_updated(query_node_number, false);
    std::vector<int> ret(query_node_number, 0);
    for (const auto& kv : query_mp)
    {
        for (const auto& info : kv.second)
        {
            ret[info.id] = info.match_node_number;
            is_updated[info.id] = true;
        }
    }
    assert(std::all_of(is_updated.begin(), is_updated.end(),
                       [](bool x) { return x; }));
    return ret;
}

std::vector<node_id_t> match_sequence_selector::get_match_sequence(
    const std::vector<int>& match_node_num)
{
    auto query_node_number = query.node_number();
    std::vector<node_id_t> ret(query_node_number);
    std::vector<priority_info> priority_infoes(query_node_number);
    std::vector<bool> is_used(query_node_number, false);
    for (size_t i = 0; i < query_node_number; i++)
    {
        auto& node = query.get_node(i);
        priority_infoes[i] = priority_info(
            i, 0, match_node_num[i], node.in_degree() + node.out_degree());
        pq.emplace(priority_infoes[i]);
    }
    std::unordered_set<node_id_t> changed_node;
    for (size_t i = 0; i < query_node_number; i++)
    {
        while (is_used[pq.top().id])
            pq.pop();
        auto id = pq.top().id;
        auto& node = query.get_node(id);
        BOOST_LOG_TRIVIAL(trace)
            << boost::format("match_order_selector::run: id [%1%], connect "
                             "[%2%], match_node_number [%3%], degree "
                             "[%4%](in:[%5%] out:[%6%])") %
                   pq.top().id % pq.top().connect %
                   (-pq.top().neg_match_node_number) % pq.top().degree %
                   node.in_degree() % node.out_degree();
        pq.pop();
        ret[i] = id;
        is_used[id] = true;
        changed_node.clear();
        for (const auto& se : node.source_edges())
        {
            auto source = se.source();
            if (is_used[source] == false)
            {
                priority_infoes[source].connect++;
                changed_node.insert(source);
            }
        }
        for (const auto& te : node.target_edges())
        {
            auto target = te.target();
            if (is_used[target] == false)
            {
                priority_infoes[target].connect++;
                changed_node.insert(target);
            }
        }
        for (auto id : changed_node)
            pq.emplace(priority_infoes[id]);
    }
    assert(
        std::all_of(is_used.begin(), is_used.end(), [](bool x) { return x; }));
    return ret;
}

std::vector<node_id_t> match_sequence_selector::run()
{
    prepare_possibility_info();
    auto node_match_number = get_query_node_match_number();
    auto ret = get_match_sequence(node_match_number);
    // ret = {253, 131, 37,  144, 242, 153, 365, 746, 152, 407, 509, 567, 321,
    // 493,
    //        771, 43,  168, 7,   31,  134, 40,  364, 98,  99,  22,  114, 229,
    //        444, 367, 127, 190, 161, 100, 41,  113, 26,  405, 34,  140, 115,
    //        406, 116, 6,   36,  39,  148, 0,   146, 35,  145, 496, 3,   25, 5,
    //        30,  107, 9,   24,  130, 391, 109, 33,  29,  119, 28,  4,   120,
    //        419, 126, 150, 128, 171, 356, 95,  154, 21,  96,  357, 91,  340,
    //        186, 614, 441, 47, 187, 615, 194, 617, 516, 482, 566, 341, 729,
    //        237, 102, 370, 366, 730, 505, 390, 133, 42,  160, 462, 151, 8, 23,
    //        32,  103, 94,  352, 457, 137, 377, 106, 385, 386, 490, 770, 353,
    //        149, 143, 38,  413, 117, 402, 141, 162, 112, 27,  401, 471, 138,
    //        420, 453, 393, 172, 560, 104, 46, 135, 166, 465, 164, 376, 577,
    //        461, 201, 643, 575, 354, 256, 535, 775, 81,  299, 501, 65,  159,
    //        538, 77,  294, 17,  82,  76,  16,  18,  2, 78,  291, 304, 20,  75,
    //        292, 136, 79,  470, 19,  85,  296, 84,  469, 193, 573, 293, 626,
    //        83,  313, 372, 716, 290, 228, 717, 310, 554, 504, 642, 80,  301,
    //        551, 619, 338, 735, 528, 189, 524, 344, 92,  345, 440, 346, 650,
    //        507, 616, 423, 442, 404, 125, 438, 170, 381, 556, 559, 165, 445,
    //        558, 437, 540, 534, 169, 155, 156, 521, 163, 74,  243, 62,  105,
    //        315, 527, 232, 59,  230, 192, 233, 510, 418, 379, 721, 231, 188,
    //        618, 542, 87,  431, 529, 719, 468, 745, 157, 508, 595, 727, 360,
    //        639, 715, 552, 239, 578, 728, 541, 640, 97,  124, 412, 238, 61,
    //        502, 167, 185, 108, 48,  123, 563, 355, 307, 86,  314, 446, 439,
    //        316, 129, 298, 88, 118, 417, 416, 562, 414, 424, 121, 101, 374,
    //        519, 734, 525, 371, 403, 305, 553, 422, 110, 179, 220, 45,  178,
    //        598, 183, 111, 318, 720, 522, 373, 421, 601, 680, 517, 571, 452,
    //        289, 506, 180, 603, 326, 285, 726, 464, 198, 51,  14,  72,  282,
    //        1,   15,  12,  60,  236, 13,  11,  234, 73,  283, 589, 58,  226,
    //        707, 382, 435, 56,  213, 223, 222, 513, 678, 221, 184, 605, 612,
    //        181, 600, 593, 570, 697, 200, 225, 50,  10,  57, 214, 681, 199,
    //        224, 54,  744, 704, 52,  206, 657, 637, 204, 651, 652, 207, 467,
    //        636, 699, 661, 776, 359, 389, 576, 388, 361, 760, 202, 631, 64,
    //        251, 763, 432, 325, 252, 460, 250, 67,  363, 266, 324, 739, 610,
    //        323, 265, 594, 69,  71,  278, 536, 277, 380, 557, 383, 705, 677,
    //        455, 240, 724, 634, 203, 647, 308, 645, 602, 241, 415, 737, 588,
    //        235, 581, 176, 286, 448, 436, 173, 44,  392, 177, 591, 227, 711,
    //        762, 583, 426, 244, 752, 635, 586, 449, 209, 670, 53,  175, 580,
    //        68,  272, 343, 271, 208, 664, 555, 702, 475, 270, 520, 665, 599,
    //        269, 498, 258, 139, 478, 582, 604, 518, 700, 288, 596, 689, 659,
    //        477, 532, 456, 476, 218, 122, 427, 731, 428, 526, 544, 656, 257,
    //        255, 55,  216, 212, 205, 568, 644, 515, 630, 196, 408, 322, 215,
    //        660, 306, 679, 219, 693, 683, 743, 676, 197, 300, 399, 495, 147,
    //        494, 49,  768, 725, 317, 261, 66,  260, 276, 262, 264, 93,  633,
    //        217, 686, 738, 549, 666, 70,  158, 274, 273, 533, 497, 327, 480,
    //        434, 195, 628, 249, 287, 473, 682, 486, 248, 63,  142, 487, 132,
    //        458, 275, 613, 764, 279, 433, 259, 246, 245, 701, 769, 466, 500,
    //        667, 174, 585, 584, 479, 384, 755, 280, 210, 268, 579, 191, 267,
    //        481, 211, 672, 674, 331, 89,  330, 410, 368, 430, 545, 653, 748,
    //        772, 358, 759, 328, 710, 492, 663, 669, 512, 606, 375, 182, 607,
    //        565, 564, 302, 329, 387, 339, 362, 254, 648, 546, 472, 709, 698,
    //        597, 398, 754, 543, 396, 447, 733, 450, 685, 622, 736, 740, 708,
    //        625, 706, 531, 411, 714, 514, 624, 673, 742, 767, 592, 491, 463,
    //        722, 295, 90,  337, 336, 334, 590, 303, 611, 695, 483, 369, 488,
    //        638, 511, 620, 632, 400, 655, 347, 712, 349, 587, 688, 263, 297,
    //        539, 654, 690, 572, 723, 641, 629, 443, 761, 474, 668, 747, 451,
    //        454, 713, 350, 530, 623, 409, 658, 773, 569, 608, 675, 312, 662,
    //        335, 333, 284, 684, 646, 561, 671, 741, 703, 485, 732, 499, 429,
    //        523, 627, 609, 503, 574, 696, 548, 342, 459, 550, 547, 319, 718,
    //        309, 537, 311, 247, 758, 757, 756, 750, 691, 351, 621, 281, 332,
    //        774, 766, 687, 489, 692, 348, 320, 649, 753, 425, 694, 397, 749,
    //        484, 378, 395, 394, 751, 765};
    return ret;
    // return get_match_sequence(possibility);
}
} // namespace slf
