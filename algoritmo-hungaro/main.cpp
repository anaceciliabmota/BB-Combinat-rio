#include <iostream>
using namespace std;

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
	bool criar_subtors = true;
	for(int i = 0; i < n;i++){
		for(int j = 0; j < n;j++){
			if(p->assignment[i][j] != 0){
				for(int k = 0; k < subtours.size();k++){
					if(subtours[k].back() == i+1){
						subtours[k].push_back(j+1);
						criar_subtors = false;
						break;
					}
				}
				if(criar_subtors){
					vector<int> v = {i+1, j+1};
					subtours.push_back(v);
				}
				criar_subtors = true;
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
vector<pair<int, int>> make_forbidden_a(vector<int> v){
	vector<pair<int, int>> forbidden_arcs;
	for(int i = 0; i < v.size()-1;i++){
		pair<int, int> fa = {
			v[i], v[i+1]	
		};
		forbidden_arcs.push_back(fa);
	}
}

void solve_hungarian(Node *node, Data *data){
	hungarian_problem_t p;
	int mode = HUNGARIAN_MODE_MINIMIZE_COST;
	double **cost = new double*[data->getDimension()];
	for (int i = 0; i < data->getDimension(); i++){
		cost[i] = new double[data->getDimension()];
		for (int j = 0; j < data->getDimension(); j++){
			cost[i][j] = data->getDistance(i,j);
		}
	}
	hungarian_init(&p, cost, data->getDimension(), data->getDimension(), mode); // Carregando o problema
	double obj_value = hungarian_solve(&p);

	node->lower_bound = obj_value;
	node->subtour = make_subtour(&p, data->getDimension());
	node->chosen = choseSubtour(node->subtour);
	node->feasible = isFeasible(node->subtour);
	node->forbidden_arcs = make_forbidden_a(node->subtour[node->chosen]);

	hungarian_free(&p);
	for (int i = 0; i < data->getDimension(); i++) delete [] cost[i];
	delete [] cost;
	
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
} 


int main(int argc, char** argv) {

	Data * data = new Data(argc, argv[1]);
	strcmp(argv[2], "BFS") == 0;
	data->readData();

	
	/*
	double **cost = new double*[data->getDimension()];
	for (int i = 0; i < data->getDimension(); i++){
		cost[i] = new double[data->getDimension()];
		for (int j = 0; j < data->getDimension(); j++){
			cost[i][j] = data->getDistance(i,j);
		}
	}*/
	/*
	hungarian_problem_t p;
	int mode = HUNGARIAN_MODE_MINIMIZE_COST;
	hungarian_init(&p, cost, data->getDimension(), data->getDimension(), mode); // Carregando o problema

	double obj_value = hungarian_solve(&p);
	cout << "Obj. value: " << obj_value << endl;

	cout << "Assignment" << endl;
	hungarian_print_assignment(&p);	
	*/
	//vector<vector<int>> subtours = make_subtour(&p, data->getDimension());
	cout << data->getBranchingStrategy()<< endl;
	list<Node> :: iterator it;
	Node root;
	solve_hungarian(&root, data);
	list<Node> tree;
	cout << "teste" << endl;
	tree.push_back(root);
	
	double upper_bound = numeric_limits<double> :: infinity();

	while(!tree.empty()){
		Node node = branchingStrategy(data->getBranchingStrategy(), tree, it);
		solve_hungarian(&node, data);
		
		if(node.lower_bound > upper_bound){
			tree.erase(it);
			continue;
		}

		
		if(node.feasible){
			upper_bound = min(upper_bound, node.lower_bound);
		}
		else{
			for(int i = 0; i < node.subtour[root.chosen].size()-1; i++){
				Node n;
				n.forbidden_arcs = root.forbidden_arcs;

				pair<int, int> forbidden_arc = {
					node.subtour[root.chosen][i],
					node.subtour[root.chosen][i+1]
				};
				n.forbidden_arcs.push_back(forbidden_arc);
				tree.push_back(n);
			}
		}
		tree.erase(it);
	}
	cout << upper_bound << endl;
	
	
	/*
	for (size_t i = 0; i < subtours.size(); i++)
	{
		for (size_t j = 0; j < subtours[i].size(); j++)
		{
			cout << subtours[i][j] << " ";	
		}
		cout << endl;
	}
	
	bool tem_subtour;
	if(subtours.size() > 1){
		tem_subtour = true;
	}*/

	/*hungarian_free(&p);
	for (int i = 0; i < data->getDimension(); i++) delete [] cost[i];
	delete [] cost;
	delete data;*/
	delete data;
	return 0;
}


