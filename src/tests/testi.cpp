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
#include<include/RandomGenerator.hpp>
#include<memory>
using namespace std;
volatile size_t fccc = 0;
//using namespace boost;
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
/*
int main() {
	int n;
	srand((unsigned int)time(NULL));
	while (cin >> n) {
		set<int> s;
		unordered_set<int> us;
		LOOP(i, 0, n) {
			s.insert(i);
		}

		system("pause");
		for (auto it = s.begin(); it != s.end(); it++) {
			us.insert(*it);
		}

		system("pause");
		clock_t t1 = clock();
		LOOP(i, 0, n) {
			int temp = abs((rand() << 12) + (rand() << 6) + rand()) % n;
			s.find(temp);

		}
		TIME_COST_PRINT("set time : ", t1);
		t1 = clock();
		LOOP(i, 0, n) {
			int temp = abs((rand() << 12) + (rand() << 6) + rand()) % n;
			us.find(temp);

		}
		TIME_COST_PRINT("us time : ", t1);
	}
	unordered_set<int> ff;
	int a;
	return 0;
	//ff.reser

}
*/
int main() {
	rg::NormalRandomGenerator generator(10, 1);
	LOOP(i, 0, 50) cout << generator.getOne() << endl;

	return 0;
}