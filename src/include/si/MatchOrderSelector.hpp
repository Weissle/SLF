#pragma once
#include <algorithm>
#include<unordered_set>
#include <queue>
#include<vector>
#include<set>
#include<map>
#include"graph/Graph.hpp"
#include"si/ThreadRelatedClass.hpp"
#include "si/si_marcos.h"
#include<utility>
#include<cmath>
#include<numeric>
using namespace std;
namespace wg {

template<typename EdgeLabel>
class MatchOrderSelector{
	using Graph = GraphS<EdgeLabel>;

	// id,poss,deg,connectNum
	using tipdc = tuple<NodeIDType,double,int,int>;
	struct tipdc_cmp{
		int operator()(const tipdc&a,const tipdc&b){
			if(get<3>(a) != get<3>(b)) return get<3>(a) < get<3>(b);
			else if(fabs(get<1>(a) - get<1>(b)) > 1e-20) return get<1>(a) > get<1>(b);
			else return get<2>(a) < get<2>(b);
		}
	};

	// better but in the worst case , time complexity is N^3.
	vector<double> CreatePossibilitiesN3(const Graph &query,const Graph &target){
		int queryMaxLabel = *max_element(query.GetLabels().begin(),query.GetLabels().end());
		int targetMaxLabel = *max_element(target.GetLabels().begin(),target.GetLabels().end());
		vector<int> queryInDegrees(query.Size()),queryOutDegrees(query.Size());
		vector<int> targetInDegrees(target.Size()),targetOutDegrees(target.Size());
		int queryInMax = 0,queryOutMax = 0;
		int targetInMax = 0,targetOutMax = 0;
		for (int i = 0; i < query.Size(); ++i){ 
			queryInDegrees[i] = query.GetInEdges(i).size();
			queryOutDegrees[i] = query.GetOutEdges(i).size();
			queryInMax = max(queryInMax,queryInDegrees[i]);
			queryOutMax = max(queryOutMax,queryOutDegrees[i]);
		}
		for (int i = 0; i < target.Size(); ++i){ 
			targetInDegrees[i] = target.GetInEdges(i).size();
			targetOutDegrees[i] = target.GetOutEdges(i).size();
			targetInMax = max(targetInMax,targetInDegrees[i]);
			targetOutMax = max(targetOutMax,targetOutDegrees[i]);
		}
		int maxLabel = max(queryMaxLabel,targetMaxLabel);
		vector<vector<int>> degreesCount[maxLabel+1];
		for (auto &one:degreesCount){
			one = vector<vector<int>>(queryInMax+1,vector<int>(queryOutMax+1,0));
		}
		for (int i = 0; i < target.Size(); ++i){ 
			int label = target.GetLabel(i);
			int inDeg = targetInDegrees[i];
			int outDeg = targetOutDegrees[i];
			inDeg = min(inDeg,queryInMax);
			outDeg = min(outDeg,queryOutMax);
			degreesCount[label][inDeg][outDeg] ++;
		}
		for (int i = 0; i <= maxLabel; ++i){ 
			for(auto j=queryInMax-1;j>=0;--j){
				degreesCount[i][j][queryOutMax] += degreesCount[i][j+1][queryOutMax];
			}
			for(auto j=queryOutMax-1;j>=0;--j){
				degreesCount[i][queryInMax][j] += degreesCount[i][queryInMax][j+1];
			}
			for(auto j=queryInMax-1;j>=0;--j){
				for(auto k=queryOutMax-1;k>=0;--k){
					degreesCount[i][j][k] = degreesCount[i][j+1][k] + degreesCount[i][j][k+1] - degreesCount[i][j+1][k+1];
				}
			}
		}
		int n = query.Size();
		vector<double> ret(n);
		for (int i = 0; i < n; ++i){ 
			ret[i] = degreesCount[query.GetLabel(i)][query.GetInDegree(i)][query.GetOutDegree(i)]/double(target.Size());
		}
		return ret;
	}
	vector<double> CreatePossibilitiesN(const Graph &query,const Graph &target){
		int maxLabel = *max_element(query.GetLabels().begin(),query.GetLabels().end());
		int maxInDeg = 0,maxOutDeg = 0;
		for (int i = 0; i < query.Size(); ++i){ 
			maxInDeg = max(maxInDeg,query.GetInDegree(i));
			maxOutDeg = max(maxOutDeg,query.GetOutDegree(i));
		}
		vector<int> labelCount(maxLabel+1,0),inDegCount(maxInDeg+1,0),outDegCount(maxOutDeg+1,0);
		for (int i = 0; i < target.Size(); ++i){ 
			auto label = target.GetLabel(i);
			auto inDeg = min(maxInDeg,target.GetInDegree(i));
			auto outDeg = min(maxOutDeg,target.GetOutDegree(i));
			if(label <= maxLabel) labelCount[label] ++;
			inDegCount[inDeg] ++;
			outDegCount[outDeg] ++;
		}
		for (int i = maxInDeg-1; i >= 0 ; --i){ 
			inDegCount[i] += inDegCount[i+1];
		}
		for (int i = maxOutDeg-1; i >= 0 ; --i){ 
			outDegCount[i] += outDegCount[i+1];
		}
		vector<double> possibility(query.Size());
		for (int i = 0; i < query.Size(); ++i){ 
			auto label = query.GetLabel(i);
			auto inDeg = query.GetInDegree(i);
			auto outDeg = query.GetOutDegree(i);
			possibility[i] = labelCount[label] * inDegCount[inDeg] * outDegCount[outDeg] / (double)(target.Size());
		}
		return possibility;
	}
public:
	MatchOrderSelector()=default;
	vector<NodeIDType> run(const Graph &query, const Graph &target, const bool slow = true){
		vector<double> possibility;
		if(slow) possibility = CreatePossibilitiesN3(query, target);
		else possibility = CreatePossibilitiesN(query, target);

		vector<NodeIDType> matchSequence;
		int n = query.Size();
		matchSequence.reserve(n);
		vector<bool> chosen(n,false);
		int connectNum[n];
		int degs[n];
		memset(connectNum,0,sizeof(connectNum));
		priority_queue<tipdc,vector<tipdc>,tipdc_cmp> pq;
		for (int i = 0; i < n; ++i){ 
			degs[i] = query.GetInDegree(i) + query.GetOutDegree(i);  
			pq.emplace(i,possibility[i],degs[i],0);
		}
		unordered_set<NodeIDType> us;
		us.reserve(n);
		while(pq.size()){
			auto temp = pq.top(); pq.pop();
			auto id = get<0>(temp);
			if (chosen[id]) continue;
			chosen[id] = true;
			matchSequence.push_back(id);
			assert(get<3>(temp) == connectNum[id]);
			us.clear();
			for (const auto &one:query.GetInEdges(id)){
				int temp_id = one.source();
				if(chosen[temp_id]) continue;
				us.insert(temp_id);
				connectNum[temp_id]++;
			}
			for (const auto &one:query.GetOutEdges(id)){
				int temp_id = one.target();
				if(chosen[temp_id]) continue;
				us.insert(temp_id);
				connectNum[temp_id]++;
			}
			for (const auto &tid:us){
				pq.emplace(tid,possibility[tid],degs[tid],connectNum[tid]);
			}
		}
		return matchSequence;

	}
	vector<NodeIDType> VF3(const Graph &query,const Graph &target){
		return run(query,target,false);
	}
	vector<NodeIDType> SI(const Graph &query,const Graph &target){
		return run(query,target,true);
	}
};

}
