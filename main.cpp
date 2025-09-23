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
    //　裏の行き止まり
    vector<Pos> deadEnds;
    vector<Pos> hazzard;
    for(int turn=0; turn<1000000; turn++){
        cerr<<"===== TURN "<<turn<<" =====\n";
        cerr<<"Adventurer at ("<<adventurer.x<<","<<adventurer.y<<")\n";

        if(adventurer.x==ti && adventurer.y==tj){
            cerr<<"Reached flower -> finish\n";
            break;
        }

        // 出力
        if (turn == 0){
            if (tj < N/2){
                tryPlaceTrent(adventurer.x - 1,adventurer.y,adventurer);
                if (tryPlaceTrent(ti-1,tj+1,adventurer)) {
                    deadEnds.push_back({ti-1,tj+1});
                }else{
                    hazzard.push_back({ti-1,tj+1});
                }
                if (tryPlaceTrent(ti,tj+1,adventurer)) {
                    deadEnds.push_back({ti,tj+1});
                }else{
                    hazzard.push_back({ti,tj+1});
                }
                if (!tryPlaceTrent(ti+1,tj,adventurer)) hazzard.push_back({ti+1,tj});
                if (!tryPlaceTrent(ti-2,tj,adventurer)) hazzard.push_back({ti-2,tj});
                hazzard.push_back({ti-1,tj+1});
                if (tryPlaceTrent(ti,tj-1,adventurer) == false) {
                    tryPlaceTrent(ti,tj-2,adventurer);
                    tryPlaceTrent(ti-1,tj-1,adventurer);
                }
            }else{
                tryPlaceTrent(adventurer.x + 1,adventurer.y,adventurer);
                if (tryPlaceTrent(ti-1,tj-1,adventurer)) {
                    deadEnds.push_back({ti-1,tj-1});
                }
                else{
                    hazzard.push_back({ti-1,tj-1});
                }
                if (tryPlaceTrent(ti,tj-1,adventurer)) {
                    deadEnds.push_back({ti,tj-1});
                }else{
                    hazzard.push_back({ti,tj-1});
                }
                if (!tryPlaceTrent(ti+1,tj,adventurer)) hazzard.push_back({ti+1,tj});
                if (!tryPlaceTrent(ti-2,tj,adventurer)) hazzard.push_back({ti-2,tj});
                hazzard.push_back({ti-1,tj-1});
                if (tryPlaceTrent(ti,tj+1,adventurer) == false) {
                    tryPlaceTrent(ti,tj+2,adventurer);
                    tryPlaceTrent(ti+1,tj+1,adventurer);
                }
            }
        }
        vector<int> deadEndFilled(deadEnds.size(), 0);
        bool placedThisTurn=false;
        for (int k = 0; k < deadEnds.size(); k++){
            if (confirmed[deadEnds[k].x][deadEnds[k].y]) deadEndFilled[k] = 1;
        }
        //最小値が0の場合、道を遮る
        for (int l = 0; l < hazzard.size(); l++){
            if (*min_element(begin(deadEndFilled), end(deadEndFilled))){
                if (adventurer.x == hazzard[l].x) {
                    tryPlaceTrent(adventurer.x, adventurer.y + 1, adventurer);
                    tryPlaceTrent(adventurer.x, adventurer.y - 1, adventurer);
                    tryPlaceTrent(adventurer.x + 2, adventurer.y, adventurer);
                    tryPlaceTrent(adventurer.x - 2, adventurer.y, adventurer);
                    placedThisTurn = true;
                }
                if (adventurer.y == hazzard[l].y) {
                    tryPlaceTrent(adventurer.x + 1, adventurer.y, adventurer);
                    tryPlaceTrent(adventurer.x - 1, adventurer.y, adventurer);
                    tryPlaceTrent(adventurer.x, adventurer.y + 2, adventurer);
                    tryPlaceTrent(adventurer.x, adventurer.y - 2, adventurer);
                    placedThisTurn = true;
                }
            }
        }
        //直近の2ターンで同じ方向に動いているなら、冒険者から見て両脇にトレントを置く
        if (adventurerPrevPrev.x != -1 && adventurerPrev.x != -1 && !placedThisTurn) {
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
