#include <bits/stdc++.h>
using namespace std;

struct Pos { int x,y; };
Pos adventurerPrevPrev{-1,-1};
Pos adventurerPrev{-1,-1};
vector<Pos> toPlace;

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
    
    // 追加: adventurer から 3 マス以内かどうか
    auto tooCloseToAdventurer = [&](int x,int y){
        return abs(x - adventurer.x) + abs(y - adventurer.y) <= 3;
    };

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
        // スタートが確認済みかどうかに応じて開始可否を判断する（必要なら）
        if (confirmed[start.x][start.y]) {
            if(cell[start.x][start.y] != '.' || hasTrent[start.x][start.y]) return dist;
            // (注) 通常スタートが冒険者なら hasTrent[start] は false のはず
        }
        queue<Pos> qu;
        dist[start.x][start.y] = 0;
        qu.push(start);
        while(!qu.empty()){
            auto p = qu.front(); qu.pop();
            for(int d=0; d<4; ++d){
                int nx = p.x + dxs[d], ny = p.y + dys[d];
                if(!inb(nx,ny)) continue;
    
                // --- ここが重要 ---
                // 「確認済みマスなら真の地形（木・確認済みトレント）を見る」
                // 「未確認マスならトレントがあっても見えない＝空きとみなす」
                if(confirmed[nx][ny]) {
                    // 確認済みマスなら木や既知のトレントで進めない
                    if(cell[nx][ny] != '.') continue;   // 木なら進めない
                    if(hasTrent[nx][ny]) continue;      // 確認済みでトレントがあれば進めない
                }
                // 未確認は '.' と仮定 → 進める（hasTrent を無視）
    
                if(dist[nx][ny] != INF) continue;
                dist[nx][ny] = dist[p.x][p.y] + 1;
                qu.push({nx,ny});
            }
        }
        return dist;
    };

    // BFS 真の地形
    auto bfsTrue=[&](Pos start, Pos block){
        vector<vector<int>> dist(N, vector<int>(N,INF));
        if(!inb(start.x,start.y)) return dist;
        if(cell[start.x][start.y] != '.' || hasTrent[start.x][start.y]) return dist;
        if(start.x==block.x && start.y==block.y) return dist; // スタートがブロックされている

        queue<Pos> qu;
        dist[start.x][start.y]=0;
        qu.push(start);

        while(!qu.empty()){
            auto p=qu.front(); qu.pop();
            for(int d=0;d<4;d++){
                int nx=p.x+dxs[d], ny=p.y+dys[d];
                if(!inb(nx,ny)) continue;
                if(nx==block.x && ny==block.y) continue; // ブロックマスを通さない
                if(cell[nx][ny] != '.') continue;
                if(hasTrent[nx][ny]) continue;
                if(dist[nx][ny]!=INF) continue;
                dist[nx][ny]=dist[p.x][p.y]+1;
                qu.push({nx,ny});
            }
        }
        return dist;
    };

    auto hasPathTrue=[&](Pos s, Pos t, int bx, int by)->bool{
        auto d = bfsTrue(s, {bx,by});
        return d[t.x][t.y] < INF;
    };

    auto tryPlaceTrent=[&](int x,int y,Pos adv)->bool{
        if(!inb(x,y)) return false;
        if(cell[x][y]!='.') return false;
        if(hasTrent[x][y]) return false;
        if(confirmed[x][y]) return false;
        if(x==adv.x&&y==adv.y) return false;
        if(!hasPathTrue(entrance,{ti,tj},x,y)) return false;
        if(!hasPathTrue(adv,{ti,tj},x,y)) return false;

        // 仮置き
        cell[x][y] = 'T';
        hasTrent[x][y] = true;

        // --- 追加チェック: 未確認かつ空き ('.') の全マスが到達可能か (bfsTrue を使用) ---
        bool ok = true;
        {
            auto dist = bfsTrue(adv, {-1,-1}); // 仮置き状態を反映した真の地形での距離
            for (int i = 0; i < N && ok; ++i) {
                for (int j = 0; j < N; ++j) {
                    // 「未確認マスのうち、木のない (= 真の地形で '.' ) マス」
                    if (!confirmed[i][j] && cell[i][j] == '.') {
                        if (dist[i][j] >= INF) { // 到達不能
                            ok = false;
                            break;
                        }
                    }
                }
            }
        }

        if (ok) {
            toPlace.push_back({x,y}); // 確定
            return true;
        } else {
            // 撤回
            cell[x][y] = '.';
            hasTrent[x][y] = false;
            return false;
        }
    };


    // 初期視界更新
    confirmed[adventurer.x][adventurer.y]=true;

    for(int turn=0; turn<1000000; turn++){
        cerr<<"===== TURN "<<turn<<" =====\n";
        cerr<<"Adventurer at ("<<adventurer.x<<","<<adventurer.y<<")\n";

        if(adventurer.x==ti && adventurer.y==tj){
            cerr<<"Reached flower -> finish\n";
            break;
        }
        // 出力
        if (turn == 0) {
            // ===== 改良版 BFS: 各 (d1,d2) シードを厳密に BFS して、最短で終端に到達した経路を採る =====
            vector<Pos> bestPath;
            int bestLen = INF;

            // 四つの角リスト（角は固定）
            vector<Pos> corners = {{0,0},{0,N-1},{N-1,0},{N-1,N-1}};

            for(int d1=0; d1<4; d1++){
                int nx1 = ti + dxs[d1], ny1 = tj + dys[d1];
                if(!inb(nx1,ny1) || cell[nx1][ny1] != '.') continue;
                for(int d2=0; d2<4; d2++){
                    // d2 は d1 と直交方向のみ
                    if(dxs[d1]*dxs[d2] + dys[d1]*dys[d2] != 0) continue;
                    int nx2 = nx1 + dxs[d2], ny2 = ny1 + dys[d2];
                    if(!inb(nx2,ny2) || cell[nx2][ny2] != '.') continue;

                    // firstStep は固定で nx1
                    Pos firstStep = {nx1, ny1};

                    // canStepLocal (step: ti=0, nx1=1, nx2=2, ...)
                    auto canStepLocal = [&](int x,int y,int step)->bool{
                        if(!inb(x,y)) return false;
                        if(cell[x][y] != '.') return false;
                        // avoid being too close to adventurer
                        if( tooCloseToAdventurer(x,y) ) return false;

                        // --- 条件: 二手目以降は (ti,tj) の隣禁止 ---
                        if(step >= 2){
                            for(int dd=0; dd<4; ++dd){
                                if(x == ti+dxs[dd] && y == tj+dys[dd]) return false;
                            }
                        }
                        // --- 条件: 三手目以降は firstStep の隣禁止 ---
                        if(step >= 3){
                            for(int dd=0; dd<4; ++dd){
                                if(x == firstStep.x+dxs[dd] && y == firstStep.y+dys[dd]) return false;
                            }
                        }
                        // --- 条件: 斜めに木が2つあるマスを避ける ---
                        int diagWood = 0;
                        int ddx[4] = {-1,-1,1,1};
                        int ddy[4] = {-1,1,-1,1};
                        for(int k=0;k<4;k++){
                            int ax = x + ddx[k], ay = y + ddy[k];
                            if(inb(ax,ay) && cell[ax][ay] == 'T') diagWood++;
                        }
                        if(diagWood >= 2) return false;

                        return true;
                    };

                    // dist / parent 初期化
                    vector<vector<int>> dist(N, vector<int>(N, INF));
                    vector<vector<Pos>> parent(N, vector<Pos>(N, {-1,-1}));

                    dist[ti][tj] = 0;
                    parent[ti][tj] = {-1,-1};
                    dist[nx1][ny1] = 1; parent[nx1][ny1] = {ti,tj};
                    dist[nx2][ny2] = 2; parent[nx2][ny2] = {nx1,ny1};

                    queue<Pos> qu;
                    qu.push({nx2,ny2});

                    bool found = false;
                    Pos endp{-1,-1};
                    int endDist = INF;

                    while(!qu.empty() && !found){
                        auto p = qu.front(); qu.pop();
                        int px = p.x, py = p.y;
                        int curd = dist[px][py];

                        for(int dd=0; dd<4; ++dd){
                            int nx = px + dxs[dd], ny = py + dys[dd];
                            if(!inb(nx,ny)) continue;
                            if(dist[nx][ny] != INF) continue; // already reached with <= dist
                            int nstep = curd + 1;
                            if(!canStepLocal(nx,ny,nstep)) continue;

                            bool isBoundary = (nx==0 || nx==N-1 || ny==0 || ny==N-1);
                            if(isBoundary){
                                // --- ここが修正の肝: 「最も近い角」を選ぶ ---
                                Pos bestCorner = corners[0];
                                int bestCdist = abs(nx - corners[0].x) + abs(ny - corners[0].y);
                                for(int ci=1; ci<4; ++ci){
                                    int cd = abs(nx - corners[ci].x) + abs(ny - corners[ci].y);
                                    if(cd < bestCdist){ bestCdist = cd; bestCorner = corners[ci]; }
                                }
                                // 角からの距離条件 (N/4 を保持)
                                if(bestCdist <= N/4){
                                    // 終点から角へ一歩・二歩進めるかチェック (角に近づく方向)
                                    int sx = (bestCorner.x > nx ? 1 : (bestCorner.x < nx ? -1 : 0));
                                    int sy = (bestCorner.y > ny ? 1 : (bestCorner.y < ny ? -1 : 0));
                                    bool ok2 = true;
                                    for(int step=1; step<=2; ++step){
                                        int xx = nx + sx*step, yy = ny + sy*step;
                                        if(!inb(xx,yy) || cell[xx][yy] != '.'){ ok2 = false; break; }
                                    }
                                    if(ok2){
                                        dist[nx][ny] = nstep;
                                        parent[nx][ny] = {px,py};
                                        endp = {nx,ny};
                                        endDist = nstep;
                                        found = true; // この seed に対する最短終点（BFS なので最短）
                                        break;
                                    } else {
                                        // 終点としては不可 -> 境界はキューに入れない
                                        continue;
                                    }
                                } else {
                                    // 角から遠すぎる -> 境界として不可
                                    continue;
                                }
                            }

                            // 通常ノード：登録してキューに入れる
                            dist[nx][ny] = nstep;
                            parent[nx][ny] = {px,py};
                            qu.push({nx,ny});
                        }
                    } // end BFS for this (d1,d2)

                    if(found){
                        // bestLen と比較 (dist を使って厳密に)
                        if(endDist < bestLen){
                            // 経路復元
                            vector<Pos> rev;
                            Pos cur = endp;
                            while(!(cur.x == -1 && cur.y == -1)){
                                rev.push_back(cur);
                                Pos pr = parent[cur.x][cur.y];
                                cur = pr;
                            }
                            reverse(rev.begin(), rev.end()); // now from ti .. endp
                            // ensure starts with (ti,tj)
                            if(rev.empty() || !(rev.front().x == ti && rev.front().y == tj)){
                                vector<Pos> tmp;
                                tmp.push_back({ti,tj});
                                tmp.push_back({nx1,ny1});
                                for(auto &pp: rev) tmp.push_back(pp);
                                rev.swap(tmp);
                            }
                            bestLen = endDist;
                            bestPath = rev;
                        }
                    }
                } // d2
            } // d1

            // debug 出力
            if(!bestPath.empty()){
                cerr << "Chosen path: ";
                for(auto &p:bestPath) cerr << "("<<p.x<<","<<p.y<<") ";
                cerr << "  length="<<bestLen<<"\n";
            } else {
                cerr << "No path found in 0-turn construction\n";
            }

            // 経路に沿ったトレント配置（経路自身には置かない） --- 既存処理を流用
            vector<Pos> extraList;
            if(!bestPath.empty()){
                Pos end = bestPath.back();
                // 角の近い方向を再計算して skip/force を決める
                vector<Pos> corners2 = {{0,0},{0,N-1},{N-1,0},{N-1,N-1}};
                Pos bestCorner = corners2[0]; int bestCdist = abs(end.x - corners2[0].x) + abs(end.y - corners2[0].y);
                for(int ci=1; ci<4; ++ci){
                    int cd = abs(end.x - corners2[ci].x) + abs(end.y - corners2[ci].y);
                    if(cd < bestCdist){ bestCdist = cd; bestCorner = corners2[ci]; }
                }
                int ex = (bestCorner.x > end.x ? 1 : (bestCorner.x < end.x ? -1 : 0));
                int ey = (bestCorner.y > end.y ? 1 : (bestCorner.y < end.y ? -1 : 0));
                Pos skip1{end.x+ex,end.y+ey};
                Pos skip2{end.x+ex*2,end.y+ey*2};
                Pos force{end.x+ex*3,end.y+ey*3};

                for(auto &p:bestPath){
                    for(int d=0;d<4;d++){
                        int nx=p.x+dxs[d], ny=p.y+dys[d];
                        if(!inb(nx,ny) || cell[nx][ny] != '.') continue;
                        // 経路自身には置かない
                        bool onPath=false;
                        for(auto &q:bestPath) if(q.x==nx && q.y==ny){ onPath=true; break; }
                        if(onPath) continue;
                        Pos cand{nx,ny};
                        if((cand.x==skip1.x&&cand.y==skip1.y)||
                           (cand.x==skip2.x&&cand.y==skip2.y)){
                            continue; // 例外的に置かない
                        }
                        tryPlaceTrent(nx,ny,adventurer);
                        // 置いたマスのさらに隣を extraList に集計
                        for(int dd=0;dd<4;dd++){
                            int nnx=nx+dxs[dd], nny=ny+dys[dd];
                            if(inb(nnx,nny)) extraList.push_back({nnx,nny});
                        }
                    }
                }
                // 三歩目は強制的に置く
                if(inb(force.x,force.y)&&cell[force.x][force.y]=='.'){
                    tryPlaceTrent(force.x,force.y,adventurer);
                }
            }
        } // end if turn==0



        bool placedThisTurn=false;
        //直近の2ターンで同じ方向に動いているなら、冒険者から見て両脇にトレントを置く
        if (adventurerPrevPrev.x != -1 && adventurerPrev.x != -1) {
            int dx1 = adventurerPrev.x - adventurerPrevPrev.x;
            int dy1 = adventurerPrev.y - adventurerPrevPrev.y;
            int dx2 = adventurer.x - adventurerPrev.x;
            int dy2 = adventurer.y - adventurerPrev.y;
            if (dx1 == dx2 && dy1 == dy2 && adventurerPrevPrev.x != 0 && adventurerPrevPrev.x != N-1 && adventurerPrevPrev.y != 0 && adventurerPrevPrev.y != N-1) { // 同じ方向に動いている
                if (dx1 != 0) { // 縦移動
                    tryPlaceTrent(adventurer.x, adventurer.y + 1, adventurer);
                    tryPlaceTrent(adventurer.x, adventurer.y - 1, adventurer);
                } else if (dy1 != 0) { // 横移動
                    tryPlaceTrent(adventurer.x + 1, adventurer.y, adventurer);
                    tryPlaceTrent(adventurer.x - 1, adventurer.y, adventurer);
                }
                placedThisTurn = true;
            }
        }
        if (!placedThisTurn){
            for (int d=0; d<4; d++){
                // adventurerの上下左右に木がないなら、その先のマスに、トレントを置けるか試す
                int nx=adventurer.x+dxs[d], ny=adventurer.y+dys[d];
                if(!inb(nx,ny)) continue;
                if(cell[nx][ny]=='T') continue; // トレントがあるなら置けない
                if(cell[nx][ny]=='#') continue; // 木があるなら置けない
                int nnx=nx+dxs[d], nny=ny+dys[d];
                tryPlaceTrent(nnx,nny,adventurer);
            }
        }

        if(toPlace.empty()){
            cout<<0<<"\n"; cout.flush(); lastPlaced=false;
            cerr<<"Output: 0\n";
        }else{
            cout<<toPlace.size();
            for(auto &p:toPlace){
                cout<<" "<<p.x<<" "<<p.y;
            }
            cout<<"\n"; cout.flush(); lastPlaced=true;
            cerr<<"Output: placed\n";
        }

        toPlace.clear();
        adventurerPrevPrev=adventurerPrev;
        adventurerPrev=adventurer;

        // --- 冒険者シミュレーション（冒険者視点: bfsProv） ---

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

        // 暫定距離 (bfsProv)
        auto distProv=bfsProv(*goal);
        int cur=distProv[adventurer.x][adventurer.y];
        if(cur>=INF){ cerr<<"WA: provisional unreachable\n"; return 0; }

        // 移動（優先度: 上→下→左→右）
        int chosen=-1;
        for(int d=0;d<4;d++){
            int nx=adventurer.x+dxs[d], ny=adventurer.y+dys[d];
            if(!inb(nx,ny)) continue;
            if(cell[nx][ny]!='.') continue; // 真の地形で木は進めない
            if(hasTrent[nx][ny]) continue;  // 実際のトレントは進めない
            if(distProv[nx][ny]<cur){ chosen=d; break; }
        }
        if(chosen==-1){ cerr<<"WA: no move\n"; return 0; }

        adventurer.x+=dxs[chosen]; adventurer.y+=dys[chosen];
        confirmed[adventurer.x][adventurer.y]=true;
        cerr<<"Move to ("<<adventurer.x<<","<<adventurer.y<<")\n";
    }

    return 0;
}