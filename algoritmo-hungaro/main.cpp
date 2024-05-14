#include <iostream>
using namespace std;

#include <cstring>
#include "data.h"
#include "hungarian.h"
#include <vector>
#include <list>
#include <limits>

struct Node{
	vector<pair<int, int>> forbidden_arcs;
	vector<vector<int>> subtour;
	double lower_bound;
	int chosen;
	bool feasible;
} typedef Node;

typedef struct Solucao{
    std::vector<int> sequencia;
    double valorObj;
} Solucao;

void exibirSolucao(Solucao *s)
{
    for(int i = 0; i < s->sequencia.size() - 1; i++)
        std::cout << s->sequencia[i] << " -> ";
    std::cout << s->sequencia.back() << std::endl;
}

void calcularValorObj(Solucao *s, Data& data){
    s->valorObj = 0;
    for(int i = 0; i < s->sequencia.size() - 1; i++)
        s->valorObj += data.getDistance(s->sequencia[i] - 1,s->sequencia[i+1] - 1);
}


vector<vector<int>> make_subtour(hungarian_problem_t *p, int n){
	vector<vector<int>> subtours;
	bool criar_subtours = true;
	for(int i = 0; i < n;i++){
		for(int j = 0; j < n;j++){
			if(p->assignment[i][j] != 0){
				for(int k = 0; k < subtours.size();k++){
					if(subtours[k].back() == i+1){
						subtours[k].push_back(j+1);
						criar_subtours = false;
						break;
					}else if(subtours[k].front() == j+1){
						subtours[k].insert(subtours[k].begin(), i+1);
						criar_subtours = false;
						break;
					}
				}
				if(criar_subtours){
					vector<int> v = {i+1, j+1};
					subtours.push_back(v);
				}
				criar_subtours = true;
			}
		}
	}
	return subtours;
}
int choseSubtour(vector<vector<int>>& subtours){
	int chosen, min_size = 99999;
	for(int i = 0; i < subtours.size();i++){
		if(subtours[i].size() < min_size){
			min_size = subtours[i].size();
			chosen = i;
		}
	}
	return chosen;
}

bool isFeasible(vector<vector<int>>& subtours){
	bool feasible = true;
	if (subtours.size() > 1){
		feasible = false;
	}	
	return feasible;	
}
 void make_forbidden_a(Node *node){
	for(int i = 0; i < node->subtour[node->chosen].size()-1;i++){
		pair<int, int> p = make_pair(node->subtour[node->chosen][i], node->subtour[node->chosen][i+1]);
	}
}

void turn_forbidden(Node *node, double ** cost){
	for(int i = 0; i < node->forbidden_arcs.size(); i++){
		cost[node->forbidden_arcs[i].first-1][node->forbidden_arcs[i].second-1] = 99999999;
		cost[node->forbidden_arcs[i].second-1][node->forbidden_arcs[i].first-1] = 99999999; 
	}
}

void solve_hungarian(Node *node, Data *data, double ** cost){
	hungarian_problem_t p;
	int mode = HUNGARIAN_MODE_MINIMIZE_COST;
	hungarian_init(&p, cost, data->getDimension(), data->getDimension(), mode); // Carregando o problema
	
	double obj_value = hungarian_solve(&p);
	hungarian_print_assignment(&p);
	node->lower_bound = obj_value;
	node->subtour = make_subtour(&p, data->getDimension());
	node->chosen = choseSubtour(node->subtour);
	node->feasible = isFeasible(node->subtour);

	hungarian_free(&p);
}
Node branchingStrategy(int branchingS, list<Node>& nodes, list<Node> :: iterator& i){
		if(branchingS == 1){
			//BFS
			i = nodes.begin();
			return nodes.front();
		}else if(branchingS == 2){
			//DFS
			i = --nodes.end();
			return nodes.back();
		}else if(branchingS == 3){
			//busca pelo menor lower_bound
			Node no;
			no.lower_bound = numeric_limits<double> :: infinity();
			for(auto it=nodes.begin();it!=nodes.end();it++){
				if((*it).lower_bound < no.lower_bound){
					no = *it;
					i = it;
				}
			}
			return no;
		}
	Node n;
	return n;
} 


int main(int argc, char** argv) {

	Data * data = new Data(argc, argv[1]);

	int branchingS;
	if(strcmp(argv[2], "BFS") == 0){
		branchingS = 1;
	}else if(strcmp(argv[2], "DFS") == 0){
		branchingS = 2;
	}else if(strcmp(argv[2], "LB") == 0){
		branchingS = 3;
	}else{
		cout << "Non valid Branching Strategy" << endl;
		return 0;
	}
	data->readData();
	
	double **cost = new double*[data->getDimension()];
	for (int i = 0; i < data->getDimension(); i++){
		cost[i] = new double[data->getDimension()];
		for (int j = 0; j < data->getDimension(); j++){
			cost[i][j] = data->getDistance(i,j);
		}
	}
	
	

	list<Node> :: iterator it;
	Node root;
	solve_hungarian(&root, data, cost);
	//turn_forbidden(&root, cost);
	list<Node> tree;
	tree.push_back(root);
	
	double upper_bound = numeric_limits<double> :: infinity();
	/*
	while(!tree.empty()){
		Node node = branchingStrategy(branchingS, tree, it);
		solve_hungarian(&node, data, cost);
		
		if(node.lower_bound > upper_bound){
			tree.erase(it);
			continue;
		}

		if(node.feasible){
			upper_bound = min(upper_bound, node.lower_bound);
		}
		else{
			for(int i = 0; i < node.subtour[node.chosen].size()-1; i++){
				Node n;
				n.forbidden_arcs = node.forbidden_arcs;
				pair<int, int> forbidden_arc = {

					node.subtour[node.chosen][i],
					node.subtour[node.chosen][i+1]
				};
				cost[node.subtour[node.chosen][i]-1][node.subtour[node.chosen][i+1]-1] = 99999999;
				n.forbidden_arcs.push_back(forbidden_arc);
				tree.push_back(n);
			}
		}
		tree.erase(it);
	}
	cout << upper_bound << endl;
	
	*/
	//solve_hungarian(&root, data, cost);
	for (size_t i = 0; i < root.subtour.size(); i++)
	{
		for (size_t j = 0; j < root.subtour[i].size(); j++)
		{
			cout << root.subtour[i][j] << " ";	
		}
		cout << endl;
	}
	
	cout << root.feasible << " " << root.chosen << endl;

	/*hungarian_free(&p);
	for (int i = 0; i < data->getDimension(); i++) delete [] cost[i];
	delete [] cost;
	delete data;*/
	for (int i = 0; i < data->getDimension(); i++) delete [] cost[i];
	delete [] cost;
	delete data;
	return 0;
}
/*
hungarian_problem_t p;
	int mode = HUNGARIAN_MODE_MINIMIZE_COST;
	hungarian_init(&p, cost, data->getDimension(), data->getDimension(), mode); // Carregando o problema

	double obj_value = hungarian_solve(&p);
	cout << "Obj. value: " << obj_value << endl;

	cout << "Assignment" << endl;
	hungarian_print_assignment(&p);	
	*/

