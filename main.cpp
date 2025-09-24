#include <bits/stdc++.h>
using namespace std;

struct Pos { int x,y; };
inline bool operator==(const Pos& a, const Pos& b){
    return a.x == b.x && a.y == b.y;
}
Pos adventurerPrevPrev{-1,-1};
Pos adventurerPrev{-1,-1};
Pos finalEndpoint = {-1,-1};      // turn==0 で決めた最終終点（壁上のセル）
Pos X_cell = {-1,-1};             // ステップ②で見つけた X（壁沿いを進んで見つける最初の '.' ）
vector<Pos> lastP;                // ステップ③で見つけた P（X -> finalEndpoint の最短経路）
Pos lastPlacedTrent = {-1,-1};    // ④で「直前にトレントを置いたマス」を追跡するため
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
                if (tj < N/2){
                    tryPlaceTrent(adventurer.x + 1,adventurer.y,adventurer);
                    tryPlaceTrent(ti+1,tj,adventurer);
                    tryPlaceTrent(ti-1,tj+1,adventurer);
                    tryPlaceTrent(ti,tj+1,adventurer);
                    tryPlaceTrent(ti-2,tj,adventurer);
                    tryPlaceTrent(ti-2,tj+2,adventurer);
                    if (tryPlaceTrent(ti,tj-1,adventurer) == false) {
                        tryPlaceTrent(ti,tj-2,adventurer);
                        tryPlaceTrent(ti+1,tj-1,adventurer);
                    }
                
                }else{
                    tryPlaceTrent(adventurer.x + 1,adventurer.y,adventurer);
                    tryPlaceTrent(ti+1,tj,adventurer);
                    tryPlaceTrent(ti-1,tj-1,adventurer);
                    tryPlaceTrent(ti,tj-1,adventurer);
                    tryPlaceTrent(ti-2,tj,adventurer);
                    tryPlaceTrent(ti-2,tj-2,adventurer);
                    if (tryPlaceTrent(ti,tj+1,adventurer) == false) {
                        tryPlaceTrent(ti,tj+2,adventurer);
                        tryPlaceTrent(ti+1,tj+1,adventurer);
                    }
                }
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
            // ===================== 新挿入ブロック： bestPath 決定後の追加戦略 =====================
            if(!bestPath.empty()){
                // ① 終点とそれに最も近い「角」を明らかにする
                finalEndpoint = bestPath.back(); // グローバルに保存
                cerr << "DEBUG: finalEndpoint = ("<<finalEndpoint.x<<","<<finalEndpoint.y<<")\n";

                // 四隅リスト
                vector<Pos> corners = {{0,0},{0,N-1},{N-1,0},{N-1,N-1}};
                // 最短の corner を求める（マンハッタン距離）
                Pos bestCorner = corners[0];
                int bestCornerDist = abs(finalEndpoint.x - corners[0].x) + abs(finalEndpoint.y - corners[0].y);
                for(int ci=1; ci<4; ++ci){
                    int cd = abs(finalEndpoint.x - corners[ci].x) + abs(finalEndpoint.y - corners[ci].y);
                    if(cd < bestCornerDist){ bestCornerDist = cd; bestCorner = corners[ci]; }
                }
                cerr << "DEBUG: bestCorner = ("<<bestCorner.x<<","<<bestCorner.y<<") dist="<<bestCornerDist<<"\n";

                // ② 終点から角とは真逆の方向に一マスずつ「壁沿い」を進んで最初に cell が T でないマスを X とする
                X_cell = {-1,-1};
                // まず終点がどの辺にいるか判定（終点は境界にいる前提）
                // 移動方向 (dxw,dyw) は「角 -> 終点」方向の逆に沿って壁を辿る方向を意味する。
                int dxw = 0, dyw = 0;
                if(finalEndpoint.x == 0){ // top edge
                    // corner が left-top (0,0) なら反対方向は右 (0,+1)
                    dyw = (bestCorner.y == 0 ? +1 : -1);
                    dxw = 0;
                } else if(finalEndpoint.x == N-1){ // bottom edge
                    dyw = (bestCorner.y == 0 ? +1 : -1);
                    dxw = 0;
                } else if(finalEndpoint.y == 0){ // left edge
                    dxw = (bestCorner.x == 0 ? +1 : -1);
                    dyw = 0;
                } else if(finalEndpoint.y == N-1){ // right edge
                    dxw = (bestCorner.x == 0 ? +1 : -1);
                    dyw = 0;
                } else {
                    // safety（終点が境界でない場合はスキップ）
                    dxw = 0; dyw = 0;
                }

                // 壁沿いに1歩ずつ進む（終点からスタート、終点自身は除外して次から）
                if(dxw!=0 || dyw!=0){
                    int cx = finalEndpoint.x + dxw;
                    int cy = finalEndpoint.y + dyw;
                    while(inb(cx,cy) && (cx==0 || cx==N-1 || cy==0 || cy==N-1)){
                        // X の条件：「cellがTでないマス(トレントも木もない)」
                        if(cell[cx][cy] == '.' && !hasTrent[cx][cy]){
                            X_cell = {cx,cy};
                            break;
                        }
                        cx += dxw; cy += dyw;
                    }
                }
                if(X_cell.x == -1){
                    cerr << "DEBUG: X not found along wall; skipping X->endpoint routing\n";
                } else {
                    cerr << "DEBUG: X_cell = ("<<X_cell.x<<","<<X_cell.y<<")\n";
                }

                // ③ Xから終点までの最短経路Pを特定する（true地形での最短。到達不可なら lastP = {}）
                lastP.clear();
                if(X_cell.x != -1){
                    // BFS (真の地形) で parent を作る（bfsTrue と同様だが parent を取る）
                    vector<vector<int>> distP(N, vector<int>(N, INF));
                    vector<vector<Pos>> parentP(N, vector<Pos>(N, {-1,-1}));
                    queue<Pos> qP;
                    // start = X_cell
                    distP[X_cell.x][X_cell.y] = 0;
                    qP.push(X_cell);
                    bool foundP = false;
                    while(!qP.empty() && !foundP){
                        auto p = qP.front(); qP.pop();
                        int px = p.x, py = p.y;
                        for(int d=0; d<4; ++d){
                            int nx = px + dxs[d], ny = py + dys[d];
                            if(!inb(nx,ny)) continue;
                            if(distP[nx][ny] != INF) continue;
                            // 真の地形条件
                            if(cell[nx][ny] != '.') continue;
                            if(hasTrent[nx][ny]) continue;
                            distP[nx][ny] = distP[px][py] + 1;
                            parentP[nx][ny] = {px,py};
                            if(nx == finalEndpoint.x && ny == finalEndpoint.y){
                                foundP = true;
                                break;
                            }
                            qP.push({nx,ny});
                        }
                    }
                    if(foundP){
                        // 復元（finalEndpoint から X へ）
                        Pos cur = finalEndpoint;
                        while(!(cur.x == -1 && cur.y == -1)){
                            lastP.push_back(cur);
                            Pos pr = parentP[cur.x][cur.y];
                            if(pr.x == -1 && pr.y == -1) break;
                            cur = pr;
                        }
                        reverse(lastP.begin(), lastP.end()); // now from X -> ... -> finalEndpoint
                        cerr << "DEBUG: P length = "<<lastP.size()<<"\n";
                    } else {
                        cerr << "DEBUG: no P from X to finalEndpoint\n";
                    }
                }

                // ④ P を花に近い順に並べた sorted_P を作る（花に近い＝(ti,tj) へのマンハッタン距離 小）
                vector<Pos> sorted_P;
                if(!lastP.empty()){
                    sorted_P = lastP;
                    sort(sorted_P.begin(), sorted_P.end(), [&](const Pos &a, const Pos &b){
                        int da = abs(a.x - ti) + abs(a.y - tj);
                        int db = abs(b.x - ti) + abs(b.y - tj);
                        if(da != db) return da < db;
                        // tie-breaker: shorter index in lastP (preserve order along path)
                        auto ita = find(lastP.begin(), lastP.end(), a);
                        auto itb = find(lastP.begin(), lastP.end(), b);
                        return (ita < itb);
                    });
                }

                // ユーティリティ: Pに含まれるか？
                auto inP = [&](int x,int y)->bool{
                    for(auto &pp: lastP) if(pp.x==x && pp.y==y) return true;
                    return false;
                };

                // 実際の配置ループ： sorted_P の各 Y について隣接マス（4方向）を試す
                bool firstPlacementDone = false;
                for(auto &Y : sorted_P){
                    // collect 4-neighbors of Y that satisfy: not in P, not touching wall, not yet trented, cell=='.'
                    vector<Pos> cand;
                    for(int d=0; d<4; ++d){
                        int nx = Y.x + dxs[d], ny = Y.y + dys[d];
                        if(!inb(nx,ny)) continue;
                        if(inP(nx,ny)) continue;              // Pに含まれない
                        if(!(nx>0 && nx<N-1 && ny>0 && ny<N-1)) continue; // 壁にも接しておらず（内部マスのみ）
                        if(cell[nx][ny] != '.') continue;
                        if(hasTrent[nx][ny]) continue;
                        cand.push_back({nx,ny});
                    }
                    // tryPlaceTrent for each candidate (order: as collected)
                    for(auto &c : cand){
                        bool ok = tryPlaceTrent(c.x, c.y, adventurer);
                        if(ok){
                            // ④全体で最初にトレントを置いたマスについての special 操作を行う
                            if(!firstPlacementDone){
                                firstPlacementDone = true;
                                lastPlacedTrent = c; // update last placed trent
                                cerr << "DEBUG: first placement at ("<<c.x<<","<<c.y<<")\n";

                                // ------------------ 【操作】ここから ------------------
                                // Z の決定：角から距離が N//3 かつ「終点とは異なる壁」にある境界上のマスを1個選ぶ
                                Pos Z = {-1,-1};
                                // 探索方針：全境界セルを走査して条件を満たすものを選ぶ（安定的に同じものを選ぶためソート的に選ぶ）
                                vector<Pos> candZ;
                                for(int x=0;x<N;++x){
                                    for(int y=0;y<N;++y){
                                        if(!(x==0 || x==N-1 || y==0 || y==N-1)) continue;
                                        if(x==finalEndpoint.x && y==finalEndpoint.y) continue; // 終点と異なること
                                        // この境界セルについて「最も近い角」を見つけ、その角からの距離が N//3 か？
                                        int bestCd = INT_MAX; Pos bestC = corners[0];
                                        for(auto &c0 : corners){
                                            int d0 = abs(x - c0.x) + abs(y - c0.y);
                                            if(d0 < bestCd){ bestCd = d0; bestC = c0; }
                                        }
                                        if(bestCd == N/3){
                                            // さらに「終点とは異なる壁沿い」にあるか：壁を表す side idx
                                            auto sideOf = [&](Pos P)->int{
                                                if(P.x==0) return 0; // top
                                                if(P.x==N-1) return 1; // bottom
                                                if(P.y==0) return 2; // left
                                                if(P.y==N-1) return 3; // right
                                                return -1;
                                            };
                                            int dCorner = abs(x - bestCorner.x) + abs(y - bestCorner.y);
                                            if(dCorner == N/3 && sideOf({x,y}) != sideOf(finalEndpoint)){
                                                candZ.push_back({x,y});
                                            }
                                        }
                                    }
                                }
                                if(!candZ.empty()){
                                    // 安定的に先頭を選ぶ（例えば lexicographic）
                                    sort(candZ.begin(), candZ.end(), [](const Pos &a, const Pos &b){
                                        if(a.x != b.x) return a.x < b.x; return a.y < b.y;
                                    });
                                    Z = candZ.front();
                                }
                                if(Z.x == -1){
                                    cerr << "DEBUG: no Z found (skip special op)\n";
                                } else {
                                    cerr << "DEBUG: chosen Z = ("<<Z.x<<","<<Z.y<<")\n";
                                    // 「直前にトレントを置いたマス」と辺または一点を共有するマス（8近傍）を Z に近い順に試す
                                    bool reachedWallPlacement = false;
                                    int safetyIter = 0;
                                    while(!reachedWallPlacement && safetyIter < 200){
                                        ++safetyIter;
                                        // collect 8-neighbors around lastPlacedTrent
                                        vector<pair<int,Pos>> around;
                                        for(int ax=-1; ax<=1; ++ax){
                                            for(int ay=-1; ay<=1; ++ay){
                                                if(ax==0 && ay==0) continue;
                                                int nx = lastPlacedTrent.x + ax, ny = lastPlacedTrent.y + ay;
                                                if(!inb(nx,ny)) continue;
                                                if(cell[nx][ny] != '.') continue;
                                                if(hasTrent[nx][ny]) continue;
                                                if(inP(nx,ny)) continue;  // ←★ Pに含まれているマスを除外
                                                int dZ = abs(nx - Z.x) + abs(ny - Z.y);
                                                around.push_back({dZ, {nx,ny}});
                                            }
                                        }
                                        if(around.empty()) break;
                                        sort(around.begin(), around.end(), [](const pair<int,Pos>& a, const pair<int,Pos>& b){
                                            if(a.first != b.first) return a.first < b.first;
                                            if(a.second.x != b.second.x) return a.second.x < b.second.x;
                                            return a.second.y < b.second.y;
                                        });
                                        bool placedThisRound = false;
                                        for(auto &pr : around){
                                            Pos cand = pr.second;
                                            if(tryPlaceTrent(cand.x, cand.y, adventurer)){
                                                // 成功したら lastPlacedTrent を更新
                                                lastPlacedTrent = cand;
                                                placedThisRound = true;
                                                cerr << "DEBUG: placed during op at ("<<cand.x<<","<<cand.y<<")\n";
                                                // 終わり条件：この cand が壁沿い（境界）ならループを抜ける
                                                if(cand.x==0 || cand.x==N-1 || cand.y==0 || cand.y==N-1){
                                                    reachedWallPlacement = true;
                                                    break;
                                                }
                                                // そうでなければ、続けてループ（next while）して cand の周囲から拡げる
                                                break; // break for -> re-evaluate around new lastPlacedTrent
                                            }
                                        }
                                        if(!placedThisRound){
                                            // 置けるものが周囲になければ終了
                                            break;
                                        }
                                    } // end while
                                    if(reachedWallPlacement) cerr << "DEBUG: wall trent reached at ("<<lastPlacedTrent.x<<","<<lastPlacedTrent.y<<")\n";
                                    else cerr << "DEBUG: op ended without reaching wall\n";
                                }
                                // ------------------ 【操作】ここまで ------------------
                            } // end if !firstPlacementDone
                            // continue with next Ys (but we do not break: we still try other neighbors as specified)
                        } // end if ok
                    } // end for candidates of Y

                    // ④全体で一度でもトレントを置いたら、既定の動作に戻る（もし要件がそうなら） —— ここは戦略次第
                    // （現状は sorted_P 全走査を続ける形にしています）
                } // end for sorted_P
            } // end if !bestPath.empty()
            // ===================== end 新挿入ブロック =====================

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