#include<iostream>
#include<unordered_set>
#include<unordered_map>
#include<algorithm>
#include<random>
#include<time.h>
#include<include\si_marcos.h>
#include<include\common.h>
#include<boost/graph/graphviz.hpp>
#include<include/argh.h>
#include<stack>
#include<queue>
using namespace std;
using namespace boost;
/*
int main(int argc,char* argv[]) {

	argh::parser cmdl({ "input" });
	cmdl.parse(argc, argv);
	string dotFilePath;
	cmdl("input") >> dotFilePath;

	// Vertex properties
	typedef property < vertex_name_t, std::string,
		property < vertex_color_t, float > > vertex_p;
	// Edge properties
	typedef property < edge_weight_t, double > edge_p;
	// Graph properties
	typedef property < graph_name_t, std::string > graph_p;
	// adjacency_list-based type
	typedef adjacency_list < vecS, vecS, directedS,
		vertex_p, edge_p, graph_p > graph_t;

	// Construct an empty graph and prepare the dynamic_property_maps.
	graph_t graph(0);
	dynamic_properties dp;

	property_map<graph_t, vertex_name_t>::type name =
		get(vertex_name, graph);
	dp.property("node_id", name);

	property_map<graph_t, vertex_color_t>::type mass =
		get(vertex_color, graph);
	dp.property("mass", mass);

	property_map<graph_t, edge_weight_t>::type weight =
		get(edge_weight, graph);
	dp.property("weight", weight);

	// Use ref_property_map to turn a graph property into a property map
	boost::ref_property_map<graph_t*, std::string>
		gname(get_property(graph, graph_name));
	dp.property("name", gname);

	// Sample graph as an std::istream;
#if 1==1	
	std::istringstream
		dotfs("digraph { graph [name=\"graphname\"]  a  c e [mass = 6.66] }");
#else
	std:ifstream dotfs;
	dotfs.open(dotFilePath.c_str());
#endif

	bool status = read_graphviz(dotfs, graph, dp, "node_id");
	assert(status && "read dot graph fail");

	const auto &nodeSet = graph.m_vertices;
	
	for (auto tempNode = nodeSet.begin(); tempNode != nodeSet.end(); ++tempNode) {
		
		cout << (*tempNode).m_property.m_value << endl;

	}


}
*/
int main() {
	map<int, int> m;
	size_t n;
	size_t labelTypeNum = 0, labelMax = 0;
	while (true) {
		cin >> n;
		if (n == -50)break;
		{
			
			auto po = m.find(n);
			if (po == m.end()) {
				labelTypeNum++;
				labelMax = max(labelMax, n);
			}
			else {
				po->second = 1;
			}
		}
	}
	cout <<( (labelTypeNum - 1 == labelMax)? "true":"false") << endl;


}