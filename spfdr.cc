#include<stdio.h>
#include<iostream>
#include<algorithm>
#include<vector>
#include<map>
#include<set>
#include<thread>
#include<stack>
#include<unistd.h>
#include<stdlib.h>
#include<sstream>
#include<sys/wait.h>
#include<sys/time.h>
#include<bits/stdc++.h>
using namespace std;

int N, M, F;
vector<vector<int> > edges;
vector<vector<pair<int, int> > > lfs;
vector<vector<int> > nfs;
vector<pair<int, int> > path(int, int, int, int);
vector<vector<int> > floyd(vector<vector<int> > &);
vector<long long> link(pair<int,int> &);
vector<long long> node(pair<int,int> &);
int dfs(int, int, int, int, vector<vector<int> > &);
#define PROCESS_NUM 64

int main(int argc, char **argv){
	N = stoi(argv[1]), M = stoi(argv[2]), F = stoi(argv[3]) % M;
	vector<pair<int, int> > f;
	edges.assign(N * M, vector<int> (N * M));
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
	for(auto &p:f)
		edges[p.first][p.second] = edges[p.second][p.first] = 1;
	long long res = 0, dis = 0, spf = 0;
	pid_t id;
	string s, a, b, c;
	int status, n;
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
			n = read(fd[i][0], line, 6000);
			line[n] = '\0';
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
	cout << (double)res / (1.0 * 2 * N * M * (N * M - 1) * N * M) << endl;
	cout << (double)dis / spf << endl;
	close_read();

	//double link failures*/
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
	cout << (double)res / (1.0 * N * M * (2 * N * M - 1) * (N * M - 1) * N * M) << endl;
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
}

vector<pair<int, int> > path(int x, int y, int s, int t){
	int ux = x,uy = (y + 1) % M;
	int dx = x,dy = (y - 1 + M) % M;
	int lx = (x - 1 + N) % N, ly = x == 0 ? (y - F + M) % M : y;
	int rx = (x + 1) % N, ry = x == N - 1 ? (y + F) % M : y;
	int pl, pr, pd, pu;
	vector<pair<int, int> > dir(4);
	if(s == x){
		if(y > t){
			pd = abs(y - t), pu = M - pd;
			if(pd < pu)
				dir[0] = {dx, dy}, dir[1] = {lx, ly}, dir[2] = {rx, ry}, dir[3] = {ux, uy};
			else
				dir[0] = {ux, uy}, dir[1] = {lx, ly}, dir[2] = {rx, ry}, dir[3] = {dx, dy};
		}
		else{	
			pu = abs(y - t), pd = M - pu;
			if(pd < pu)
				dir[0] = {dx, dy}, dir[1] = {lx, ly}, dir[2] = {rx, ry}, dir[3] = {ux, uy};
			else
				dir[0] = {ux, uy}, dir[1] = {lx, ly}, dir[2] = {rx, ry}, dir[3] = {dx, dy};
		}
	}
	else if(s > x){
		pl = N - abs(s - x) + min(abs((y - F + M) % M - t), M - abs((y - F + M) % M - t));
		pr = abs(s - x) + min(abs(y - t), M - abs(y - t));
		if(y > t){
			if(pl < pr){
				int v = min(abs((y - F + M) % M - t), M - abs((y - F + M) % M - t));
				if(v == 0)
					dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};
				else{
					if((y - F + M) % M > t){
						if(abs((y - F + M) % M - t) < M - abs((y - F + M) % M - t))
							dir[0] = {lx, ly}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {rx, ry};
						else
							dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};
					}
					else{
						if(abs((y - F + M) % M - t) < M - abs((y - F + M) % M - t))
							dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};
						else
							dir[0] = {lx, ly}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {rx, ry};
					}
				}
			}
			else{
				if(abs(y - t) < M - abs(y - t))
					dir[0] = {rx, ry}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {lx, ly};
				else
					dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};
			}
		}
		else{
			if(pl < pr){
				int v = min(abs((y - F + M) % M - t), M - abs((y - F + M) % M - t));
				if(v == 0)
					dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};
				else{
					if((y - F + M) % M > t){
						if(abs((y - F + M) % M - t) < M - abs((y - F + M) % M - t))
							dir[0] = {lx, ly}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {rx, ry};
						else
							dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};
					}
					else{
						if(abs((y - F + M) % M - t) < M - abs((y - F + M) % M - t))
							dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};
						else
							dir[0] = {lx, ly}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {rx, ry};
					}
				}
			}
			else{
				int v = min(abs(y - t), M - abs(y - t));
				if(v == 0)
					dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};
				else{
					if(abs(y - t) < M - abs(y - t))
						dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};
					else
						dir[0] = {rx, ry}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {lx, ly};
				}
			}
		}
	}
	else{
		pl = abs(s - x) + min(abs(y - t), M - abs(y - t));
		pr = N - abs(s - x) + min(abs((y + F) % M - t), M - abs((y + F) % M - t));
		if(y > t){
			if(pl < pr){
				if(abs(y - t) < M - abs(y - t))
					dir[0] = {lx, ly}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {rx, ry};
				else
					dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};
			}
			else{
				if((y + F) % M > t){
					if(abs((y + F) % M - t) < M - abs((y + F) % M - t))
						dir[0] = {rx, ry}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {lx, ly};
					else
						dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};
				}
				else{
					if(abs((y + F) % M - t) < M - abs((y + F) % M - t))
						dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};
					else
						dir[0] = {rx, ry}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {lx, ly};
				}
			}
		}
		else{
			if(pl < pr){
				int v = min(abs(y - t), M - abs(y - t));
				if(v == 0)
					dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};
				else{
					if(abs(y - t) < M - abs(y - t))
						dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};
					else
						dir[0] = {lx, ly}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {rx, ry};
				}
			}
			else{
				int v = min(abs((y + F) % M - t), M - abs((y + F) % M - t));
				if(v == 0)
					dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};
				else{
					if((y + F) % M > t){
						if(abs((y + F) % M - t) < M - abs((y + F) % M - t))
							dir[0] = {rx, ry}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {lx, ly};
						else
							dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};
					}
					else{
						if(abs((y + F) % M - t) < M - abs((y + F) % M - t))
							dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};
						else
							dir[0] = {rx, ry}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {lx, ly};
					}
				}
			}
		}
	}
	if(x + 1 == s){
		if(y == t)
			dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};	
		else if(y > t){
			if(y - t < M - (y - t))
				dir[0] = {rx, ry}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {lx, ly};
			else
				dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};
		}
		else{
			if(t - y < M - (t - y))
				dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};
			else
				dir[0] = {rx, ry}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {lx, ly};
		}
	}
	if(s + 1 == x){
		if(y == t)
			dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};	
		else if(y > t){
			if(y - t < M - (y - t))
				dir[0] = {lx, ly}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {rx, ry};
			else
				dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};
		}
		else{
			if(t - y < M - (t - y))
				dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};
			else
				dir[0] = {lx, ly}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {rx, ry};
		}
	}
	if(x == 0 && s == N - 1){
		int y1 = (y - F + M) % M;
		if(y1 == t)
			dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};	
		else if(y1 > t){
			if(y1 - t < M - (y1 - t))
				dir[0] = {lx, ly}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {rx, ry};	
			else
				dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};
		}
		else{
			if(t - y1 < M - (t - y1))
				dir[0] = {lx, ly}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {rx, ry};	
			else
				dir[0] = {lx, ly}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {rx, ry};
		}
	}
	if(x == N - 1 && s == 0){
		int y1 = (y + F) % M;
		if(y1 == t)
			dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};	
		else if(y1 > t){
			if(y1 - t < M - (y1 - t))
				dir[0] = {rx, ry}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {lx, ly};	
			else 
				dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};
		}
		else{
			if(t - y1 < M - (t - y1))
				dir[0] = {rx, ry}, dir[1] = {ux, uy}, dir[2] = {dx, dy}, dir[3] = {lx, ly};	
			else
				dir[0] = {rx, ry}, dir[1] = {dx, dy}, dir[2] = {ux, uy}, dir[3] = {lx, ly};
		}
	}
	return dir;
}

vector<vector<int> > floyd(vector<vector<int> > &arr){
	int n = N * M;
    vector<vector<int> > f(n, vector<int> (n, 0x3f3f3f3f));
    for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++)
            if(arr[i][j])
                f[i][j] = 1;
    for(int k = 0; k < n; k++)
        for(int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                f[i][j] = min(f[i][j], f[i][k] + f[k][j]);
    return f;
}

vector<long long> link(pair<int, int> &p){
	int l = p.first, r = p.second;
	long long res = 0, dis = 0, spf = 0;
	for(int i = l; i < r; i++){
		vector<pair<int, int> > &arr = lfs[i];
		for(auto &x : arr)
			edges[x.first][x.second] = edges[x.second][x.first] = 0;
		vector<vector<int> > d = floyd(edges);
		for(int a = 0; a < N * M; a++)
			for(int b = 0; b < N * M; b++)
				if(a != b){
					int v = dfs(a % N, a / N, b % N, b / N, edges);
					if(v < 30){
						res++;
						dis += v;
						spf += d[a][b];
					}
				}
		for(auto &x:arr)
			edges[x.first][x.second] = edges[x.second][x.first] = 1;
	}
	return {res, dis, spf};
}


int dfs(int x, int y, int s, int t, vector<vector<int> > &cur){
	vector<pair<int, int> > st;
	st.push_back({x, y});
	while(!(st.back().first == s && st.back().second == t)){
		if(st.size() > 30)
			break;
		x = st.back().first;
		y = st.back().second;
		int sx = -1, sy = -1, j = -1;
		if(st.size() >= 2)
			sx = st[st.size() - 2].first, sy = st[st.size() - 2].second;
		vector<pair<int, int> > dir = path(x, y, s, t);	
		int nx, ny;
		for(int i = 0; i < 4; i++){
			nx = dir[i].first, ny = dir[i].second;
			if(!(nx == sx && ny == sy) && cur[y * N + x][ny * N + nx]){
				j = i;	
				break;
			}
		}
		if(nx == dir[3].first && ny == dir[3].second)
			nx = sx, ny = sy;
		if(sx == s && x == ((sx - 1 + N) % N) && j > 1)
			nx = sx, ny = sy;
		st.push_back({nx, ny});
	}
	return st.size() - 1;
}

vector<long long> node(pair<int,int> &p){
	int l = p.first, r = p.second;
	long long res = 0, dis = 0, spf = 0;
	for(int i = l; i < r; i++){
		vector<int> &arr = nfs[i];
		set<int> st;
		vector<pair<int, int> > egs;
		for(auto &x : arr){
			st.insert(x);
			for(int j = 0; j < N * M; j++)
				if(edges[x][j])
					egs.push_back({x, j});
		}
		for(auto &x : egs)
			edges[x.first][x.second] = edges[x.second][x.first] = 0;
		vector<vector<int> > d = floyd(edges);
		for(int a = 0; a < N * M; a++){
			if(st.find(a) != st.end())	
				continue;
			for(int b = 0; b < N * M; b++){
				if(st.find(b) != st.end())
					continue;
				if(a != b){
					int v = dfs(a % N, a / N, b % N, b / N, edges);
					if(v < 30){
						res++;
						dis += v;
						spf += d[a][b];
					}
				}
			}
		}
		for(auto &x : egs)
			edges[x.first][x.second] = edges[x.second][x.first] = 1;
	}
	return {res, dis, spf};
}
