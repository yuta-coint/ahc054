#include <bits/stdc++.h>
using namespace std;

struct Pos { int x,y; };

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, ti, tj;
    if(!(cin >> N >> ti >> tj)) return 0;

    vector<string> b(N);
    for(int i=0;i<N;i++) cin >> b[i];

    vector<vector<char>> cell(N, vector<char>(N));
    for(int i=0;i<N;i++) for(int j=0;j<N;j++) cell[i][j] = b[i][j];

    // qリスト（ローカルテスト用）
    vector<Pos> q;
    q.reserve(N*N-1);
    for(int k=0;k<N*N-1;k++){
        int qi,qj; cin >> qi >> qj;
        q.push_back({qi,qj});
    }

    auto inb=[&](int x,int y){ return 0<=x && x<N && 0<=y && y<N; };

    vector<vector<bool>> confirmed(N, vector<bool>(N,false));
    vector<vector<bool>> hasTrent(N, vector<bool>(N,false));

    Pos entrance{0, N/2};
    Pos adventurer=entrance;
    confirmed[adventurer.x][adventurer.y]=true;

    bool lastPlaced=false;
    int qidx=0;
    optional<Pos> goal;

    int dxs[4]={-1,1,0,0};
    int dys[4]={0,0,-1,1};
    const int INF=1e9;

    // BFS 暫定地図（未確認はすべて空きマス扱い）
    auto bfsProv = [&](Pos start){
        vector<vector<int>> dist(N, vector<int>(N, INF));
        if(!inb(start.x,start.y)) return dist;
        if (confirmed[start.x][start.y]) {
            if(cell[start.x][start.y] != '.' || hasTrent[start.x][start.y]) return dist;
            cerr << "Tree is on bfsProv start (" << start.x << "," << start.y << ")\n";
        }
        queue<Pos> qu;
        dist[start.x][start.y] = 0;
        qu.push(start);
        while(!qu.empty()){
            auto p = qu.front(); qu.pop();
            for(int d=0; d<4; ++d){
                int nx = p.x + dxs[d], ny = p.y + dys[d];
                if(!inb(nx,ny)) continue;
                if(hasTrent[nx][ny]) continue;
                if(confirmed[nx][ny]) {
                    if(cell[nx][ny] != '.') continue; // 木なら進めない
                }
                // 未確認は '.' と仮定 → 進める
                if(dist[nx][ny] != INF) continue;
                dist[nx][ny] = dist[p.x][p.y] + 1;
                qu.push({nx,ny});
            }
        }
        return dist;
    };

    // BFS 真の地形
    auto bfsTrue=[&](Pos start){
        vector<vector<int>> dist(N, vector<int>(N,INF));
        if(!inb(start.x,start.y)) return dist;
        if(cell[start.x][start.y]!='.' || hasTrent[start.x][start.y]) return dist;
        queue<Pos> qu; dist[start.x][start.y]=0; qu.push(start);
        while(!qu.empty()){
            auto p=qu.front(); qu.pop();
            for(int d=0;d<4;d++){
                int nx=p.x+dxs[d], ny=p.y+dys[d];
                if(!inb(nx,ny)) continue;
                if(cell[nx][ny]!='.') continue;
                if(hasTrent[nx][ny]) continue;
                if(dist[nx][ny]!=INF) continue;
                dist[nx][ny]=dist[p.x][p.y]+1;
                qu.push({nx,ny});
            }
        }
        return dist;
    };

    auto hasPathTrue=[&](Pos s,Pos t,int bx,int by)->bool{
        auto d=bfsTrue(s);
        return d[t.x][t.y]<INF;
    };

    auto canPlaceTrent=[&](int x,int y,Pos adv)->bool{
        if(!inb(x,y)) return false;
        if(cell[x][y]!='.') return false;
        if(hasTrent[x][y]) return false;
        if(confirmed[x][y]) return false;
        if(x==adv.x&&y==adv.y) return false;
        if(!hasPathTrue(entrance,{ti,tj},x,y)) return false;
        if(!hasPathTrue(adv,{ti,tj},x,y)) return false;
        return true;
    };

    // 初期視界更新
    for(int d=0; d<4; d++){
        int x=adventurer.x, y=adventurer.y;
        while(inb(x,y)){
            confirmed[x][y]=true;
            if(cell[x][y]=='T') break;
            x+=dxs[d]; y+=dys[d];
        }
    }

    for(int turn=0; turn<1000000; turn++){
        cerr<<"===== TURN "<<turn<<" =====\n";
        cerr<<"Adventurer at ("<<adventurer.x<<","<<adventurer.y<<")\n";

        if(adventurer.x==ti && adventurer.y==tj){
            cerr<<"Reached flower -> finish\n";
            break;
        }

        // 出力
        vector<Pos> toPlace;
        if (!lastPlaced && confirmed[ti][tj]) {
            int distF = abs(adventurer.x - ti) + abs(adventurer.y - tj);
            for (int d = 0; d < 4; d++) {
                int nx = ti + dxs[d], ny = tj + dys[d];
                if (!inb(nx, ny)) continue;
                if (abs(adventurer.x - nx) + abs(adventurer.y - ny) == distF - 1) {
                    if (canPlaceTrent(nx, ny, adventurer)) {
                        // ★ 経路長の変化をチェック
                        auto before = bfsTrue(adventurer);
                        int distBefore = before[ti][tj];
                        hasTrent[nx][ny] = true;
                        auto after = bfsTrue(adventurer);
                        int distAfter = after[ti][tj];
                        hasTrent[nx][ny] = false;
        
                        if (distAfter < INF && distAfter >= distBefore) {
                            // 経路を潰さず、むしろ遠回りさせられるときのみ置く
                            toPlace.push_back({nx, ny});
                            break;
                        }
                    }
                }
            }
        }

        if(toPlace.empty()){
            cout<<0<<"\n"; cout.flush(); lastPlaced=false;
            cerr<<"Output: 0\n";
        }else{
            cout<<toPlace.size();
            for(auto &p:toPlace){
                cout<<" "<<p.x<<" "<<p.y;
                hasTrent[p.x][p.y]=true;
            }
            cout<<"\n"; cout.flush(); lastPlaced=true;
            cerr<<"Output: placed\n";
        }

        // --- 審判シミュレーション ---

        // 視界更新
        for(int d=0; d<4; d++){
            int x=adventurer.x, y=adventurer.y;
            while(inb(x,y)){
                confirmed[x][y]=true;
                if(cell[x][y]=='T') break;
                x+=dxs[d]; y+=dys[d];
            }
        }

        // 目的地決定
        if(confirmed[ti][tj]){
            goal=Pos{ti,tj};
            cerr<<"Goal=flower\n";
        } else{
            auto distProv=bfsProv(adventurer);
            bool found=false;
            while(qidx<(int)q.size()){
                auto cand=q[qidx];
                if(!confirmed[cand.x][cand.y] && distProv[cand.x][cand.y]<INF){
                    goal=cand;
                    found=true;
                    cerr<<"Goal from q: ("<<cand.x<<","<<cand.y<<")" << distProv[cand.x][cand.y] << "\n";
                    break;
                }
                qidx++;
            }
            if(!found){ cerr<<"WA: no reachable unknown\n"; return 0; }
        }

        // 暫定距離
        auto distProv=bfsProv(*goal);
        int cur=distProv[adventurer.x][adventurer.y];
        if(cur>=INF){ cerr<<"WA: provisional unreachable\n"; return 0; }

        // 移動（優先度: 上→下→左→右）
        int chosen=-1;
        for(int d=0;d<4;d++){
            int nx=adventurer.x+dxs[d], ny=adventurer.y+dys[d];
            if(!inb(nx,ny)) continue;
            if(cell[nx][ny]!='.') continue;
            if(hasTrent[nx][ny]) continue;
            cerr << "  Check move to (" << nx << "," << ny << ") distProv=" << distProv[nx][ny] << "\n";
            cerr << confirmed[7][10] << cell[7][10] << hasTrent[7][10] << "\n";
            if(distProv[nx][ny]<cur){ chosen=d; break; }
        }
        if(chosen==-1){ cerr<<"WA: no move\n"; return 0; }

        adventurer.x+=dxs[chosen]; adventurer.y+=dys[chosen];
        confirmed[adventurer.x][adventurer.y]=true;
        cerr<<"Move to ("<<adventurer.x<<","<<adventurer.y<<")\n";
    }

    return 0;
}
