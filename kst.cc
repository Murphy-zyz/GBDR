#include<iostream>
#include<queue>
#include<vector>
#include<algorithm>
#include<string.h>
#include<set>
#include<unistd.h>
#include<stdlib.h>
#include<sstream>
#include<sys/wait.h>
#include<sys/time.h>
using namespace std;

int tot, ans, n, cnt;
int head[100001], des[100001], edge[100001], nxt[100001], vis[100001], pre[100001];
void add_edge(int, int, int);
bool bfs(int, int);
int edge_connectivity(vector<vector<int> > &, int, int);
vector<vector<pair<int, int> > > k_edge_disjoint(vector<vector<int> > &, int);
bool check(vector<vector<int> > &, int, int, int, int);

int N, M, F;
vector<vector<int> > raw_edge;
vector<vector<int> > floyd(vector<vector<int> > &);
vector<vector<vector<pair<int, int> > > > trees;
vector<vector<pair<int, int> > > lfs;
vector<vector<int> > nfs;
vector<long long> link(pair<int, int> &);
vector<long long> node(pair<int, int> &);
int dfs(int, int, vector<vector<int> > &);
#define PROCESS_NUM 64

int main(int argc, char **argv){
	N = stoi(argv[1]), M = stoi(argv[2]), F = stoi(argv[3]) % M;
	vector<pair<int, int> > f;
	for(int i = 0; i < N * M; i++)
		for(int j = i + 1; j < N * M; j++){
			int x = i % N, y = i / N, s = j % N, t = j / N;
			if(x == s){
				if(y > t){
					if(y == t + 1 || (t == 0 && y == M - 1))
						f.push_back({i + 1, j + 1});
				}
				else{
					if(y == t - 1 || (y == 0 && t == M - 1))	
						f.push_back({i + 1, j + 1});
				}
			}
			else if(s > x){
				if((s == x + 1 && y == t) || (s == N - 1 && x == 0 && ((t + F) % M == y)))
					f.push_back({i + 1, j + 1});
			}
			else{
				if((s == x - 1 && y == t) || (x == N - 1 && s == 0 && ((y + F) % M == t)))
					f.push_back({i + 1, j + 1});
			}
		}
	n = N * M;
	raw_edge.assign(n + 1, vector<int> (n + 1));
	for(auto &p : f)
		raw_edge[p.first][p.second] = raw_edge[p.second][p.first] = 1;
	trees.resize(n + 1);
    for(int i = 1; i <= n; i++){
        vector<vector<int> > edges = raw_edge;
        trees[i] = k_edge_disjoint(edges, i);
    }
	long long res = 0, dis = 0, spf = 0;
	pid_t id;
	string s, a, b, c;
	int status, num;
	char line[6001];
	int fd[PROCESS_NUM][2], k = 0;
	auto make_pipe = [&](){
		for(int i = 0; i < PROCESS_NUM; i++)
			pipe(fd[i]);
	};
	auto close_write = [&](){
		for(int i = 0; i < PROCESS_NUM; i++)
			close(fd[i][1]);
	};
	auto close_read = [&](){
		for(int i = 0; i < PROCESS_NUM; i++)
			close(fd[i][0]);
	};
	auto get_data = [&](){
		for(int i = 0; i < PROCESS_NUM; i++){
			num = read(fd[i][0], line, 6000);
			line[num] = '\0';
			s = line;
			stringstream ss(s);
			while(ss >> a >> b >> c)
				res += stoll(a), dis += stoll(b), spf += stoll(c);
		}
	};
	auto write_data = [&](vector<long long> &arr){
		a = to_string(arr[0]) + " ", b = to_string(arr[1]) + " ", c = to_string(arr[2]) + " ";
		write(fd[k][1], a.c_str(), a.size());
		write(fd[k][1], b.c_str(), b.size());
		write(fd[k][1], c.c_str(), c.size());
	};
	auto init = [&](){
		if(!lfs.empty()) lfs.clear();
		if(!nfs.empty()) nfs.clear();
		res = 0, spf = 0, dis = 0, k = 0;
		make_pipe();
	};

	//single link failure
	init();

	for(int i = 0; i < 2 * N * M; i++)
		lfs.push_back({f[i]});
	for(int i = 0; i < lfs.size(); i += lfs.size() / PROCESS_NUM + 1){
		pair<int, int> p = {i, min(lfs.size(), i + lfs.size() / PROCESS_NUM + 1)};
		if((id = fork()) == 0){
			close_read();
			vector<long long> arr = link(p);
			write_data(arr);
			exit(0);
		}
		k++;
	}

	close_write();
	while(wait(&status) > 0);
	get_data();
	cout << (double)res / (2.0 * N * M * (N * M - 1) * N * M) << endl;
	cout << (double)dis / spf << endl;
	close_read();
	
	//double link failures
	init();

	for(int i = 0; i < 2 * N * M; i++)
		for(int j = i + 1; j < 2 * N * M; j++)
			lfs.push_back({f[i], f[j]});
	for(int i = 0; i < lfs.size(); i += lfs.size() / PROCESS_NUM + 1){
		pair<int, int> p = {i, min(lfs.size(), i + lfs.size() / PROCESS_NUM + 1)};
		if((id = fork()) == 0){
			close_read();
			vector<long long> arr = link(p);
			write_data(arr);
			exit(0);
		}
	}

	close_write();
	while(wait(&status) > 0);
	get_data();
	cout << (double)res / (1.0 * N * M * (2 * N * M - 1) * (N * M - 1) * N * M) << endl;
	cout << (double)dis / spf << endl;
	close_read();
	
	//triple link failures
	init();

	for(int i = 0; i < 2 * N * M; i++)
		for(int j = i + 1; j < 2 * N * M; j++)
			for(int k = j + 1; k < 2 * N * M; k++)
				lfs.push_back({f[i], f[j], f[k]});
	for(int i = 0; i < lfs.size(); i += lfs.size() / PROCESS_NUM + 1){	
		pair<int, int> p = {i, min(lfs.size(), i + lfs.size() / PROCESS_NUM + 1)};
		if((id = fork()) == 0){
			close_read();
			vector<long long> arr = link(p);
			write_data(arr);
			exit(0);
		}
		k++;
	}

	close_write();
	while(wait(&status) > 0);
	get_data();
	cout << (double)res / (2.0 * N * M * (2 * N * M - 1) * (2 * N * M - 2) / 6 * (N * M - 1) * N * M) << endl;
	cout << (double)dis / spf<<endl;
	close_read();

	//single node failure
	init();

	for(int i = 1; i <= N * M; i++)
		nfs.push_back({i});
	for(int i = 0; i < nfs.size(); i += nfs.size() / PROCESS_NUM + 1){	
		pair<int, int> p = {i, min(nfs.size(), i + nfs.size() / PROCESS_NUM + 1)};
		if((id = fork()) == 0){
			close_read();
			vector<long long> arr = node(p);
			write_data(arr);
			exit(0);
		}
		k++;
	}

	close_write();
	while(wait(&status) > 0);
	get_data();
	cout << (double)res / (1.0 * N * M * (N * M - 1) * (N * M - 2)) << endl;
	cout << (double)dis / spf << endl;
	close_read();

	//double node failures
	init();

	for(int i = 1; i <= N * M; i++)
		for(int j = i + 1; j <= N * M; j++)
			nfs.push_back({i, j});
	for(int i = 0; i < nfs.size(); i += nfs.size() / PROCESS_NUM + 1){	
		pair<int, int> p = {i, min(nfs.size(), i + nfs.size() / PROCESS_NUM + 1)};
		if((id = fork()) == 0){
			close_read();
			vector<long long> arr = node(p);
			write_data(arr);
			exit(0);
		}
		k++;
	}

	close_write();
	while(wait(&status) > 0);
	get_data();
	cout << (double)res / (1.0 * N * M * (N * M - 1) / 2 * (N * M - 2) * (N * M - 3)) << endl;
	cout << (double)dis / spf << endl;
	close_read();

	//triple node failures
	init();

	for(int i = 1; i <= N * M; i++)
		for(int j = i + 1; j <= N * M; j++)
			for(int k = j + 1; k <= N * M; k++)
				nfs.push_back({i, j, k});
	for(int i = 0; i < nfs.size(); i += nfs.size() / PROCESS_NUM + 1){	
		pair<int, int> p = {i, min(nfs.size(), i + nfs.size() / PROCESS_NUM + 1)};
		if((id = fork()) == 0){
			close_read();
			vector<long long> arr = node(p);
			write_data(arr);
			exit(0);
		}
		k++;
	}

	close_write();
	while(wait(&status) > 0);
	get_data();
	cout << (double)res / (1.0 * N * M * (N * M - 1) * (N * M - 2) / 6 * (N * M - 3) * (N * M - 4)) << endl;
	cout << (double)dis / spf << endl;
	close_read();
}

void add_edge(int x, int y, int w){
	edge[++tot] = w, des[tot] = y, nxt[tot] = head[x], head[x] = tot;
}


bool bfs(int s, int t){
	queue<int> q;
	memset(vis, 0, sizeof(vis));
	q.push(s);
	vis[s] = 1;
	int flow = 0x3f3f3f3f;
	while(!q.empty()){
		int top = q.front();
		q.pop();
		for(int i = head[top]; i; i = nxt[i])
			if(edge[i]){
				int nbr = des[i];
				if(vis[nbr])
					continue;
				vis[nbr] = 1;
				flow = min(flow, edge[i]);
				q.push(nbr);
				pre[nbr] = i;
				if(nbr == t){
					int x = t;
					while(x != s){
						int e = pre[x];
						edge[e] -= flow;
						edge[e ^ 1] += flow;
						x = des[e ^ 1];
					}
					ans += flow;
					return true;
				}
			}
	
	}
	return false;
}

int edge_connectivity(vector<vector<int> > &edges, int s, int t){
	memset(nxt, 0, sizeof(nxt));
	memset(head, 0, sizeof(head));
	memset(edge, 0, sizeof(edge));
	memset(des, 0, sizeof(des));
	ans = 0, tot = 1;
	int u = n + 1;
	vector<vector<int> > f(n + 1, vector<int> (n + 1));
	for(int i = 1; i < n + 1; i++)
		for(int j = 1; j < n + 1; j++)
			if(edges[i][j] && !f[i][j] && edges[j][i] && !f[j][i]){
				f[i][j] = f[j][i] = 1;
				add_edge(i, j, 1);
				add_edge(j, i, 0);
				add_edge(j, u, 1);
				add_edge(u, j, 0);
				add_edge(u, i, 1);
				add_edge(i, u++, 0);
			}
			else if(edges[i][j] && !f[i][j]){
				add_edge(i, j, 1);
				add_edge(j, i, 0);
				f[i][j] = 1;
			}
	while(bfs(s, t))
		;
	return ans;
}


vector<vector<pair<int,int> > > k_edge_disjoint(vector<vector<int> > &edges, int r){
	int k = 0x3f3f3f3f;
	for(int i = 1; i <= n; i++)
		if(i != r)
			k = min(k, edge_connectivity(edges, r, i));
	vector<vector<pair<int,int> > > f;
	for(int i = 1; i <= k; i++){
		vector<pair<int, int> > tree_edges;
		set<int> tree_nodes;
		tree_nodes.insert(r);
		while(tree_nodes.size() < n){
			for(auto &node : tree_nodes){
				for(int j = 1; j <= n; j++)
					if(j != node && edges[node][j] && tree_nodes.find(j) == tree_nodes.end()){
						if(!check(edges, node, j, k - i, r))
							continue;
						edges[node][j] = 0;
						tree_nodes.insert(j);
						tree_edges.push_back({node, j});
					}
			}
		}
		for(auto &p : tree_edges)
			swap(p.first, p.second);
		f.push_back(tree_edges);
	}
	return f;
}


bool check(vector<vector<int> > &edges, int x, int y, int k, int r){
	memset(nxt, 0, sizeof(nxt));
	memset(head, 0, sizeof(head));
	memset(edge, 0, sizeof(edge));
	memset(des, 0, sizeof(des));
	ans = 0, tot = 1;
	int u = n + 1;
	vector<vector<int> > f(n + 1, vector<int> (n + 1));
	for(int i = 1; i < n + 1; i++)
		for(int j = 1; j < n + 1; j++)
			if(edges[i][j] && !f[i][j] && edges[j][i] && !f[j][i]){
				f[i][j] = f[j][i] = 1;
				add_edge(i, j, 1);
				add_edge(j, i, 0);
				add_edge(j, u, 1);
				add_edge(u, j, 0);
				add_edge(u, i, 1);
				add_edge(i, u++, 0);
			}
			else if(edges[i][j] && !f[i][j]){
				add_edge(i, j, 1);
				add_edge(j, i, 0);
				f[i][j] = 1;
			}
	for(int i = 0; i < k; i++){
		add_edge(x, u, 1);
		add_edge(u, x, 0);
		add_edge(u, r, 1);
		add_edge(r, u++, 0);
	}
	while(bfs(x, y))
		;
	return ans >= k + 1;
}

vector<vector<int> > floyd(vector<vector<int> > &edges){
    vector<vector<int> > f(n + 1, vector<int> (n + 1, 0x3f3f3f3f));
    for(int i = 1; i < n + 1; i++)
        for(int j = 1; j < n + 1; j++)
            if(edges[i][j])
                f[i][j] = 1;
    for(int k = 1; k <= n; k++)
        for(int i = 1; i <= n; i++)
            for(int j = 1; j <= n; j++)
                f[i][j] = min(f[i][j], f[i][k] + f[k][j]);
    return f;
}

vector<long long> link(pair<int, int> &p){
	long long res = 0, spf = 0, dis = 0;
	int l = p.first, r = p.second;
	for(int i = l; i < r; i++){
		vector<pair<int, int> > &fs = lfs[i];
		for(auto &x : fs)
			raw_edge[x.first][x.second] = raw_edge[x.second][x.first] = 0;
		vector<vector<int> > d = floyd(raw_edge);
		for(int a = 1; a <= N * M; a++)
			for(int b = 1; b <= N * M; b++)
				if(a != b){
					int v = dfs(a, b, raw_edge);
					if(v < 61){
						res++;
						dis += v;
						spf += d[a][b];
					}
				}
		for(auto &x : fs)
			raw_edge[x.first][x.second] = raw_edge[x.second][x.first] = 1;
	}
	return {res, dis, spf};
}

vector<long long> node(pair<int,int> &p){
	long long res = 0, spf = 0, dis = 0;
	int l = p.first, r = p.second;
	for(int i = l; i < r; i++){
		vector<int> &fs = nfs[i];
		set<int> st;
		vector<pair<int, int> > edges;
		for(auto &x : fs){
			st.insert(x);
			for(int i = 1; i <= N * M; i++)
				if(raw_edge[x][i])
					edges.push_back({x, i});
		}
		for(auto &e : edges)
			raw_edge[e.first][e.second] = raw_edge[e.second][e.first] = 0;
		vector<vector<int> > d = floyd(raw_edge);
		for(int a = 1; a <= N * M; a++){
			if(st.find(a) != st.end())
				continue;
			for(int b = 1; b <= N * M; b++){
				if(st.find(b) != st.end())
					continue;
				if(a != b){
					int v = dfs(a, b, raw_edge);
					if(v < 61){
						res++;
						dis += v;
						spf += d[a][b];
					}
				}
			}
		}
		for(auto &e:edges)
			raw_edge[e.first][e.second] = raw_edge[e.second][e.first] = 1;
	}
	return {res, dis, spf};
}

int dfs(int cur, int end, vector<vector<int> > &edges){	
	vector<int> st;
	st.push_back(cur);
	vector<vector<pair<int, int> > > tr = trees[end];
	while(st.back() != end){
		if(st.size() > 61)
			break;
		cur = st.back();
		if(st.size() == 1){
			for(int i = 0; i < 4; i++){
				vector<pair<int, int> > &p = tr[i];
				bool flag = false;
				for(auto &pr : p)
					if(pr.first == cur && edges[cur][pr.second]){
						st.push_back(pr.second);
						flag = true;
						break;
					}
				if(flag)
					break;
			}		
		}
		else{
			int pre = st[st.size() - 2], j;
			for(int i = 0; i < 4; i++){
				vector<pair<int, int> > &p = tr[i];
				bool flag = false;
				for(auto &pr : p)
					if(pr.first == pre && pr.second == cur){
						flag = true;
						break;
					}
				if(flag){
					j = i;
					break;
				}
			}
			for(auto &pr : tr[j])
				if(pr.first == cur && edges[pr.second][cur]){
					st.push_back(pr.second);
					break;
				}
			if(cur == st.back()){
				for(int i = 0; i < 4; i++){
					vector<pair<int, int> > &p = tr[i];
					bool flag = false;
					for(auto &pr : p)
						if(pr.first == cur && edges[cur][pr.second]){
							st.push_back(pr.second);
							flag = true;
							break;
						}
					if(flag)
						break;
				}	
			}
		}
	}
	return st.size() - 1;
}
