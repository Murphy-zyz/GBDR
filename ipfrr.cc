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
#include<bits/stdc++.h>
using namespace std;

#define PROCESS_NUM 64
int fa[1001][1001], lk_backup[1001][1001], nd_backup[1001][1001];

int n, N, M, F;
vector<vector<int> > raw_edge, nbrs;
vector<vector<int> > floyd(vector<vector<int> > &);
vector<vector<pair<int, int> > > lfs;
vector<vector<int> > nfs;
vector<long long> link(pair<int, int> &);
vector<long long> node(pair<int, int> &);
int dfs(int, int, vector<vector<int> > &, int);
void dijk_pq(int);
void link_backup();
void node_backup();

int main(int argc, char **argv){
	N = stoi(argv[1]), M = stoi(argv[2]), F = stoi(argv[3]) % M;
	vector<pair<int,int> > f;
	for(int i = 0; i < N * M; i++)
		for(int j = i + 1; j < N * M; j++){
			int x = i % N, y = i / N, s = j % N, t = j / N;
			if(x == s){
				if(y > t){
					if(y == t + 1 || (t == 0 && y == M - 1))
						f.push_back({i, j});
				}
				else{
					if(y == t - 1 || (y == 0 && t == M - 1))	
						f.push_back({i, j});
				}
			}
			else if(s > x){
				if((s == x + 1 && y == t) || (s == N - 1 && x == 0 && ((t + F) % M == y)))
					f.push_back({i, j});
			}
			else{
				if((s == x - 1 && y == t) || (x == N - 1 && s == 0 && ((y + F) % M == t)))
					f.push_back({i, j});
			}
		}
	n = N * M;
	raw_edge.assign(n,  vector<int> (n));
	nbrs.resize(n);
	memset(fa, -1, sizeof(fa));
	memset(lk_backup, -1, sizeof(lk_backup));
	memset(nd_backup, -1, sizeof(nd_backup));
	for(auto &p:f){
		raw_edge[p.first][p.second] = raw_edge[p.second][p.first] = 1;
		nbrs[p.first].push_back(p.second);
		nbrs[p.second].push_back(p.first);
	}
	link_backup();
	node_backup();
	
	long long res = 0, dis = 0, spf = 0;
	string s, a, b, c;
	pid_t id;
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
		pair<int, int> p = {i, min(lfs.size(),i + lfs.size() / PROCESS_NUM + 1)};
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
	cout << (double)dis / spf<<endl;
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
		k++;
	}
	close_write();
	while(wait(&status) > 0);
	get_data();
	cout << (double)res / (N * M * (2.0 * N * M - 1) * (N * M - 1) * N * M) << endl;
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
	cout << (double)dis / spf << endl;
	close_read();

	//single node failure
	init();

	for(int i = 0; i < N * M; i++)
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

	for(int i = 0; i < N * M; i++)
		for(int j = i + 1; j < N * M; j++)
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

	for(int i = 0; i < N * M; i++)
		for(int j = i + 1; j < N * M; j++)
			for(int k = j + 1; k < N * M; k++)
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
	cout << (double)res / (1.0 * N * M * (N * M - 1) * (N * M - 2) / 6 * (N * M - 3) * (N * M - 4))<<endl;
	cout << (double)dis / spf << endl;
	close_read();
}

void dijk_pq(int x){
	priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int> > > pq;
	pq.push({0, x});
	int vis[N * M], dis[N * M];
	memset(vis, 0, sizeof(vis));
	memset(dis, 0x3f, sizeof(dis));
	while(!pq.empty()){
		auto top = pq.top();
		int u = top.first, v = top.second;
		pq.pop();
		if(vis[v])
			continue;
		vis[v] = 1;
		for(auto &i : nbrs[v])
			if(!vis[i] && dis[i] > u + 1){
				dis[i] = u + 1;
				fa[x][i] = v;
				pq.push({u + 1, i});
			}			
	}
}

void link_backup(){
	int vis[N * M];
	memset(vis, 0, sizeof(vis));
	function<void(int, int, int, int)> mark = [&](int cur, int pre, int r, int v){
		vis[cur] = v;
		for(auto &nbr : nbrs[cur])
			if(fa[r][nbr] == cur)
				mark(nbr, cur, r, v);
	};
	function<void(int, int, int)> cal_link_backup = [&](int cur, int pre, int r){
		if(cur != r && lk_backup[r][cur] == -1){
			mark(r, -1, r, 0);
			mark(cur, pre, r, 1);
			queue<int> q;
			q.push(cur);
			int end = -1;
			while(!q.empty()){
				int x = q.front();
				for(auto &nbr : nbrs[x])
					if(nbr != pre && !vis[nbr]){
						end = x;
						lk_backup[r][x] = nbr;
						break;
					}
				if(end != -1)
					break;
				q.pop();
				for(auto &nbr : nbrs[x])
					if(vis[nbr] && fa[r][nbr] == x)
						q.push(nbr);	
			}
			int u = end;
			while(u != cur){
				lk_backup[r][fa[r][u]] = u;
				u = fa[r][u];
			}	
		}
		for(auto &nbr : nbrs[cur])
			if(fa[r][nbr] == cur)
				cal_link_backup(nbr, cur, r);
	};
	for(int i = 0; i < N * M; i++){
		dijk_pq(i);
		cal_link_backup(i, -1, i);	
	}
}

void node_backup(){
	int vis[N * M];
	memset(vis, 0, sizeof(vis));	
	function<void(int, int, int, int)> mark = [&](int cur, int pre, int r, int v){
		vis[cur] = v;
		for(auto &nbr : nbrs[cur])
			if(fa[r][nbr] == cur)
				mark(nbr, cur, r, v);
	};
	function<int(int, int, int)> cnt = [&](int cur, int pre, int r)->int{
		int ans = vis[cur];
		for(auto &nbr : nbrs[cur])
			if(fa[r][nbr] == cur)
				ans += cnt(nbr, cur, r);
		return ans;
	};
	function<void(int, int, int)> cal_node_backup = [&](int cur, int pre, int r){
		if(cur != r){
			mark(r, -1, r, 0);	
			mark(cur, pre, r, 1);
			for(auto &nbr : nbrs[cur])
				if(fa[r][nbr] == cur && !vis[nbr])
					mark(nbr, cur, r, 0);
			while(cnt(cur, pre, r) > 1){
				int end = -1;
				queue<int> q;
				q.push(cur);
				while(!q.empty()){
					int x = q.front();
					q.pop();
					for(auto &nbr : nbrs[x])
						if(x != cur && !vis[nbr]){
							end = x;
							nd_backup[r][x] = nbr;
							break;
						}
					if(end != -1)
						break;	
					for(auto &nbr : nbrs[x])
						if(fa[r][nbr] == x && vis[nbr])
							q.push(nbr);
				}
				int u = end;
				while(fa[r][u] != cur){
					nd_backup[r][fa[r][u]] = u;
					u = fa[r][u];
				}
				mark(u, fa[r][u], r, 0);
			}
		}
		for(auto &nbr : nbrs[cur])
			if(fa[r][nbr] == cur)
				cal_node_backup(nbr, cur, r);
	};
	for(int i = 0; i < N * M; i++){
		dijk_pq(i);
		cal_node_backup(i, -1, i);
	}	
}

vector<vector<int> > floyd(vector<vector<int> > &edges){
    vector<vector<int> > f(n, vector<int> (n, 0x3f3f3f3f));
    for(int i = 0; i < n; i++)
        for(int j = 0; j < n;j++)
            if(edges[i][j])
                f[i][j] = 1;
    for(int k = 0; k < n; k++)
        for(int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                f[i][j] = min(f[i][j], f[i][k] + f[k][j]);
    return f;
}

vector<long long> link(pair<int,int> &p){
	long long res = 0, spf = 0, dis = 0;
	int l = p.first, r = p.second;
	for(int i = l; i < r; i++){
		vector<pair<int, int> > &fs = lfs[i];
		for(auto &x : fs)
			raw_edge[x.first][x.second] = raw_edge[x.second][x.first] = 0;
		vector<vector<int> > d = floyd(raw_edge);
		for(int a = 0; a < N * M; a++)
			for(int b = 0; b < N * M; b++)
				if(a != b){
					int v = dfs(a, b, raw_edge, 0);
					if(v < 61){
						res++;
						dis += v;
						spf += d[a][b];
					}
				}
		for(auto &x:fs)
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
			for(int i = 0; i < N * M; i++)
				if(raw_edge[x][i])
					edges.push_back({x, i});
		}
		for(auto &e : edges)
			raw_edge[e.first][e.second] = raw_edge[e.second][e.first] = 0;
		vector<vector<int> > d = floyd(raw_edge);
		for(int a = 0; a < N * M; a++){
			if(st.find(a) != st.end())
				continue;
			for(int b = 0; b < N * M; b++){
				if(st.find(b) != st.end())
					continue;
				if(a != b){
					int v = dfs(a, b, raw_edge, 1);
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

int dfs(int cur, int end, vector<vector<int> > &edges, int v){	
	vector<int> st;
	st.push_back(cur);
	while(st.back() != end){
		if(st.size() > 61)
			break;
		cur = st.back();
		if(st.size() == 1){
			if(edges[cur][fa[end][cur]])
				st.push_back(fa[end][cur]);
			else{
				if(v == 0)
					if(edges[cur][lk_backup[end][cur]])
						st.push_back(lk_backup[end][cur]);
					else
						return 61;
				if(v == 1)
					if(edges[cur][nd_backup[end][cur]])
						st.push_back(nd_backup[end][cur]);
					else
						return 61;
			}
		}
		else{
			if(edges[cur][fa[end][cur]] && st[st.size() - 2] != fa[end][cur])
				st.push_back(fa[end][cur]);
			else{	
				if(v == 0)
					if(edges[cur][lk_backup[end][cur]])
						st.push_back(lk_backup[end][cur]);
					else
						return 61;
				if(v == 1)
					if(edges[cur][nd_backup[end][cur]])
						st.push_back(nd_backup[end][cur]);
					else
						return 61;
			}
		}
	}
	return st.size() - 1;
}
