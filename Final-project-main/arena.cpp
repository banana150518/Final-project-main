#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <sstream>
#include <deque>
#include <optional>
#include <cstdint>
 
using namespace std;
 
struct Point {
    int x = 0, y = 0;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
};
 
struct EffectPopup {
    Point tile;
    string label;
    sf::Color color;
    float alpha = 255.f;
    float offsetY = 0.f;
};
 
class Player {
public:
    Player(string name_, char icon_, sf::Color color_)
        : name(name_), icon(icon_), color(color_) {}
 
    bool isAlive()         const { return alive; }
    bool isPoisoned()      const { return poisoned; }
    int  getHp()           const { return hp; }
    bool canMove()         const { return movable; }
    int  getPoisonTurns() const { return poisonTurns; }
    Point getPosition()   const { return pos; }
    string getName()      const { return name; }
    char getIcon()        const { return icon; }
    sf::Color getColor()  const { return color; }
 
    void setPoisoned(bool p, int turns = 3) { poisoned = p; poisonTurns = turns; }
    void setMovable(bool m) { movable = m; }
    void setBlockDiceTarget(int t) { blockTarget = t; }
    void setPosition(Point p) { pos = p; }
    void revive() { hp=100; alive=true; poisoned=false; movable=true; poisonTurns=0; blockTarget=0; }
 
    int rollDice() { return (rand()%6)+1; }
 
    // ── แก้ไข: รับ Board& เพื่อเช็คกำแพง ────────────────────────────
    // forward declare เป็น template เพื่อหลีกเลี่ยง circular dependency
    template<typename BoardT>
    bool move(const string& dir, BoardT& board) {
        if (!movable) return false;
        Point n = pos;
        if (dir=="W") n.y--; if (dir=="S") n.y++;
        if (dir=="A") n.x--; if (dir=="D") n.x++;
        if (n.x>=0 && n.x<10 && n.y>=0 && n.y<10 && board.isWalkable(n.x,n.y)){
            pos=n; return true;
        }
        return false;
    }
 
    void takeDamage(int dmg) {
        hp -= dmg;
        if (hp<=0){hp=0;alive=false;}
        if (hp>100) hp=100;
    }
    void heal(int amount) {
        hp += amount;
        if (hp>100) hp=100;
        if (hp>0&&!alive) alive=true;
    }
    void tickPoison() {
        if (!poisoned) return;
        takeDamage(2); poisonTurns--;
        if (poisonTurns<=0) poisoned=false;
    }
    bool tryUnblock() {
        if ((rand()%20)+1 > blockTarget){ movable=true; blockTarget=0; return true; }
        return false;
    }
    float getHpPercent() const { return hp/100.0f; }
 
private:
    string name; char icon; sf::Color color;
    int hp=100;
    Point pos{0,0};
    bool alive=true, poisoned=false, movable=true;
    int poisonTurns=0, blockTarget=0;
};
 
class Board;
static bool boardWalkable(Board& b,int x,int y);
static Point randomNearby(Board& board,Point centre);
 
class Tile {
public:
    virtual ~Tile(){}
    virtual string onStep(Player& p,vector<Player>& all,Board& board) = 0;
    virtual bool isWalkable() const { return true; }
    virtual string getName()  const { return "?"; }
    virtual int getType()     const { return 0; }
    virtual string overlayLine1() const { return ""; }
    virtual string overlayLine2() const { return ""; }
    virtual sf::Color overlayColor() const { return sf::Color::White; }
    virtual sf::Color borderColor()  const { return sf::Color(0,0,0,0); }
};
 
class NormalTile : public Tile {
public:
    string onStep(Player&,vector<Player>&,Board&) override { return ""; }
    string getName() const override { return "Normal"; }
    int getType()    const override { return 0; }
};
 
class WallTile : public Tile {
public:
    string onStep(Player&,vector<Player>&,Board&) override { return ""; }
    bool isWalkable() const override { return false; }
    string getName()  const override { return "Wall"; }
    int getType()     const override { return 1; }
    string overlayLine1() const override { return "WALL"; }
    sf::Color overlayColor() const override { return sf::Color(220,120,120); }
    sf::Color borderColor()  const override { return sf::Color(180,60,60,200); }
};
 
class TrapTile : public Tile {
public:
    string onStep(Player& p,vector<Player>&,Board&) override {
        p.takeDamage(10);
        p.setPoisoned(true,3);
        p.heal(5);
        return "TRAP  -10HP / Poison 3T / +5HP";
    }
    string getName()  const override { return "Trap"; }
    int getType()     const override { return 2; }
    string overlayLine1() const override { return "TRAP"; }
    string overlayLine2() const override { return "-10HP/PSN/+5HP"; }
    sf::Color overlayColor() const override { return sf::Color(255,160,80); }
    sf::Color borderColor()  const override { return sf::Color(255,120,30,230); }
};
 
class AntidoteTile : public Tile {
public:
    string onStep(Player& p,vector<Player>&,Board& board) override {
        p.setPoisoned(false,0);
        Point dest = randomNearby(board,p.getPosition());
        p.setPosition(dest);
        return "ANTIDOTE  Cure + Teleport->(" +
               to_string(dest.x)+","+to_string(dest.y)+")";
    }
    string getName()  const override { return "Antidote"; }
    int getType()     const override { return 3; }
    string overlayLine1() const override { return "ANTIDOTE"; }
    string overlayLine2() const override { return "Cure+Teleport"; }
    sf::Color overlayColor() const override { return sf::Color(80,255,160); }
    sf::Color borderColor()  const override { return sf::Color(60,255,130,230); }
};
 
static void applyCard(int card,Player& p,vector<Player>& all,string& msg){
    switch(card){
        case 1:{ int tot=0,cnt=0;
            for(auto& o:all) if(o.getName()!=p.getName()&&o.isAlive()){o.takeDamage(5);tot+=5;cnt++;}
            if(cnt>0) p.heal(tot/cnt);
            msg="Life Steal +5/คน, ฮีล "+to_string(cnt>0?tot/cnt:0); break;}
        case 2: p.heal(10); msg="ฮีล +10 HP"; break;
        case 3: for(auto& o:all) if(o.getName()!=p.getName()&&o.isAlive()) o.setPoisoned(true,3);
                msg="พิษทุกคน 3 เทิร์น"; break;
        case 4: for(auto& o:all) if(o.getName()!=p.getName()&&o.isAlive()){o.setMovable(false);o.setBlockDiceTarget(15);}
                msg="บล็อกทุกคน"; break;
        case 5: p.takeDamage(10); msg="-10 HP ตัวเอง"; break;
        case 6:{ int drain=(int)(all.size()-1)*2; p.takeDamage(min(drain,p.getHp()));
            for(auto& o:all) if(o.getName()!=p.getName()&&o.isAlive()) o.heal(2);
            msg="Drain ตัวเอง / คนอื่น +2"; break;}
        case 7: p.setMovable(false); p.setBlockDiceTarget(15); msg="บล็อกตัวเอง"; break;
        default: msg="???";
    }
}
 
class QuestionableTile : public Tile {
public:
    string onStep(Player& p,vector<Player>& all,Board&) override {
        int card=(rand()%7)+1; string msg;
        applyCard(card,p,all,msg);
        return "? CARD  "+msg;
    }
    string getName()  const override { return "?"; }
    int getType()     const override { return 4; }
    string overlayLine1() const override { return "? CARD"; }
    string overlayLine2() const override { return "Random"; }
    sf::Color overlayColor() const override { return sf::Color(255,240,80); }
    sf::Color borderColor()  const override { return sf::Color(255,230,50,230); }
};
 
// ════════════════════════════════════════════════════════════════════
class Board {
public:
    Board(){ reset(); }
    ~Board(){ clear(); }
 
    void reset(){
        clear();
        grid.resize(10,vector<Tile*>(10,nullptr));
        for(int i=0;i<10;i++) for(int j=0;j<10;j++) grid[i][j]=new NormalTile();
        setTile(2,2,new TrapTile()); setTile(7,1,new TrapTile());
        setTile(4,6,new TrapTile()); setTile(8,8,new TrapTile());
        setTile(5,5,new AntidoteTile()); setTile(1,4,new AntidoteTile());
        setTile(8,3,new AntidoteTile());
        setTile(7,3,new QuestionableTile()); setTile(3,7,new QuestionableTile());
        setTile(0,9,new QuestionableTile()); setTile(9,0,new QuestionableTile());
        setTile(1,8,new WallTile()); setTile(3,3,new WallTile());
        setTile(6,6,new WallTile()); setTile(0,5,new WallTile());
        setTile(9,4,new WallTile());
    }
 
    void setTile(int x,int y,Tile* t){ delete grid[y][x]; grid[y][x]=t; }
    Tile* getTile(int x,int y) const { return (x>=0&&x<10&&y>=0&&y<10)?grid[y][x]:nullptr; }
    bool isWalkable(int x,int y) const {
        if(x<0||x>=10||y<0||y>=10) return false;
        return grid[y][x]->isWalkable();
    }
    int getTileType(int x,int y) const {
        if(x<0||x>=10||y<0||y>=10) return 1;
        return grid[y][x]->getType();
    }
    Tile* getTileObj(int x,int y) const {
        if(x<0||x>=10||y<0||y>=10) return nullptr;
        return grid[y][x];
    }
 
    void shrink(int layer){
        if(layer>=5) return;
        for(int i=layer;i<10-layer;i++){
            setTile(layer,i,new WallTile());   setTile(9-layer,i,new WallTile());
            setTile(i,layer,new WallTile());   setTile(i,9-layer,new WallTile());
        }
    }
 
private:
    vector<vector<Tile*>> grid;
    void clear(){ for(auto& row:grid) for(auto* t:row) delete t; grid.clear(); }
};
 
static bool boardWalkable(Board& b,int x,int y){ return b.isWalkable(x,y); }
static Point randomNearby(Board& board,Point centre){
    vector<Point> cands;
    for(int dy=-1;dy<=1;dy++) for(int dx=-1;dx<=1;dx++){
        if(dx==0&&dy==0) continue;
        int nx=centre.x+dx,ny=centre.y+dy;
        if(board.isWalkable(nx,ny)) cands.push_back({nx,ny});
    }
    if(cands.empty()) return centre;
    return cands[rand()%cands.size()];
}
 
// ════════════════════════════════════════════════════════════════════
class GameEngine {
public:
    GameEngine(){
        players={
            Player("Boss",  'B',sf::Color(100,200,255)),
            Player("Potter",'P',sf::Color(255,200,80)),
            Player("Pim",   'M',sf::Color(180,120,255)),
            Player("Nammon",'N',sf::Color(80,220,150))
        };
        resetGame();
    }
 
    void resetGame(){
        board.reset();
        for(auto& p:players) p.revive();
        vector<Point> taken;
        for(int i=0;i<4;i++){
            Point pos;
            do{ pos={rand()%10,rand()%10}; }
            while(!board.isWalkable(pos.x,pos.y)||find(taken.begin(),taken.end(),pos)!=taken.end());
            taken.push_back(pos); players[i].setPosition(pos);
        }
        turnCount=0; shrinkLayer=1; stepsLeft=0;
        hasRolled=false; gameOver=false; winner="";
        currentPlayer=rand()%4;
        activeGasZones.clear(); logLines.clear(); popups.clear();
        addLog("เริ่มเกม! ผู้เล่นแรก: "+players[currentPlayer].getName());
    }
 
    void addLog(const string& msg){
        logLines.push_front(msg);
        if(logLines.size()>10) logLines.pop_back();
    }
 
    void spawnPopup(Point tile,const string& label,sf::Color col){
        popups.push_back({tile,label,col,255.f,0.f});
    }
 
    void rollDice(){
        if(gameOver) return;
        Player& p=players[currentPlayer];
        if(!p.canMove()){
            if(p.tryUnblock()) addLog(p.getName()+" หลุดจากสตัน!");
            else{ addLog(p.getName()+" ถูกสตัน ข้ามเทิร์น"); nextTurn(); return; }
        }
        int roll=p.rollDice(); stepsLeft=roll; hasRolled=true;
        addLog(p.getName()+" ทอยได้ "+to_string(roll)+" ก้าว");
    }
 
    void movePlayer(const string& dir){
        if(gameOver||!hasRolled||stepsLeft<=0) return;
        Player& p=players[currentPlayer];
 
        // ── แก้ไข: เช็คว่า next tile เดินได้ก่อน ────────────────────
        Point cur=p.getPosition(), next=cur;
        if(dir=="W") next.y--; if(dir=="S") next.y++;
        if(dir=="A") next.x--; if(dir=="D") next.x++;
 
        if(!board.isWalkable(next.x,next.y)){
            addLog("เดินไม่ได้! มีกำแพงหรือขอบบอร์ด");
            return;
        }
 
        if(!p.move(dir,board)){ addLog("เดินไม่ได้!"); return; }
        stepsLeft--;
 
        Tile* t=board.getTile(p.getPosition().x,p.getPosition().y);
        if(t){
            string result=t->onStep(p,players,board);
            if(!result.empty()){
                addLog(p.getName()+": "+result);
                spawnPopup(p.getPosition(), result,
                           t->overlayColor()==sf::Color::White
                               ? sf::Color(255,220,80) : t->overlayColor());
            }
        }
 
        if(stepsLeft==0) nextTurn();
    }
 
    void nextTurn(){
        if(gameOver) return;
        hasRolled=false; stepsLeft=0;
        for(int i=0;i<4;i++) if(i!=currentPlayer&&players[i].isAlive()) players[i].tickPoison();
        turnCount++;
        int totalHp=0;
        for(auto& p:players) if(p.isAlive()) totalHp+=p.getHp();
        if(totalHp<200&&shrinkLayer<5){
            board.shrink(shrinkLayer);
            addLog("บอร์ดหด! ชั้น "+to_string(shrinkLayer)); shrinkLayer++;
        }
        if(turnCount%(max(1,(int)players.size()))==0) randomGasZone();
        checkWinner(); if(gameOver) return;
        do{ currentPlayer=(currentPlayer+1)%4; } while(!players[currentPlayer].isAlive());
        addLog("เทิร์น: "+players[currentPlayer].getName());
    }
 
    void randomGasZone(){
        if(rand()%2==0) return;
        int ox=rand()%8,oy=rand()%8;
        activeGasZones.push_back({ox,oy});
        addLog("แก๊สพิษที่ ("+to_string(ox)+","+to_string(oy)+")!");
        for(auto& p:players){
            if(!p.isAlive()) continue;
            Point pos=p.getPosition();
            if(pos.x>=ox&&pos.x<=ox+2&&pos.y>=oy&&pos.y<=oy+2){
                p.takeDamage(5);
                addLog(p.getName()+" โดนแก๊ส -5 HP!");
                spawnPopup(pos,"GAS  -5HP",sf::Color(255,180,60));
            }
        }
    }
 
    void checkWinner(){
        vector<Player*> alive;
        for(auto& p:players) if(p.isAlive()) alive.push_back(&p);
        if(alive.size()==1){
            gameOver=true; winner=alive[0]->getName()+" ชนะ!";
            addLog("จบเกม - "+winner);
        }
    }
 
    void tickPopups(float dt){
        for(auto& pop:popups){
            pop.offsetY -= 40.f*dt;
            pop.alpha   -= 180.f*dt;
        }
        popups.erase(remove_if(popups.begin(),popups.end(),
            [](const EffectPopup& p){ return p.alpha<=0.f; }),popups.end());
    }
 
    vector<Player>& getPlayers()         { return players; }
    int getCurrentPlayer()         const { return currentPlayer; }
    bool isGameOver()              const { return gameOver; }
    string getWinner()             const { return winner; }
    int getStepsLeft()             const { return stepsLeft; }
    bool hasRolledYet()            const { return hasRolled; }
    deque<string>& getLog()              { return logLines; }
    Board& getBoard()                    { return board; }
    vector<pair<int,int>>& getGasZones() { return activeGasZones; }
    vector<EffectPopup>& getPopups()     { return popups; }
 
private:
    vector<Player> players;
    Board board;
    int turnCount=0,shrinkLayer=1,currentPlayer=0,stepsLeft=0;
    bool hasRolled=false,gameOver=false;
    string winner;
    deque<string> logLines;
    vector<pair<int,int>> activeGasZones;
    vector<EffectPopup> popups;
};
 
// ════════════════════════════════════════════════════════════════════
void drawRoundedRect(sf::RenderWindow& win,float x,float y,float w,float h,
                     sf::Color fill,sf::Color outline,float thick,float=10.f){
    sf::RectangleShape r(sf::Vector2f(w,h));
    r.setPosition({x,y}); r.setFillColor(fill);
    r.setOutlineColor(outline); r.setOutlineThickness(thick); win.draw(r);
}
void drawHpBar(sf::RenderWindow& win,float x,float y,float w,float h,float pct,
               sf::Color bg,sf::Color fg){
    sf::RectangleShape b(sf::Vector2f(w,h)); b.setPosition({x,y}); b.setFillColor(bg); win.draw(b);
    sf::RectangleShape f(sf::Vector2f(w*pct,h)); f.setPosition({x,y}); f.setFillColor(fg); win.draw(f);
}
void drawText(sf::RenderWindow& win,sf::Font& font,const string& str,float x,float y,
              unsigned size,sf::Color col){
    sf::Text t(font,sf::String::fromUtf8(str.begin(),str.end()),size);
    t.setFillColor(col); t.setPosition({x,y}); win.draw(t);
}
 
void drawPlayerCard(sf::RenderWindow& win,sf::Font& font,const Player& p,
                    float x,float y,float w,float h,bool isCurrent){
    sf::Color bc=isCurrent?sf::Color(255,210,50):sf::Color(80,85,120);
    drawRoundedRect(win,x,y,w,h,sf::Color(25,28,48,230),bc,isCurrent?2.5f:1.f);
    sf::CircleShape av(22.f); av.setPosition({x+12,y+12});
    av.setFillColor(sf::Color(40,45,70)); av.setOutlineThickness(2.f); av.setOutlineColor(p.getColor()); win.draw(av);
    drawText(win,font,string(1,p.getIcon()),x+23,y+16,22,p.getColor());
    drawText(win,font,p.getName(),x+68,y+12,18,sf::Color(230,230,240));
    drawText(win,font,"("+to_string(p.getPosition().x)+","+to_string(p.getPosition().y)+")",x+68,y+33,13,sf::Color(160,165,190));
    drawText(win,font,"HP",x+12,y+60,13,sf::Color(180,185,200));
    drawText(win,font,to_string(p.getHp())+"/100",x+w-60,y+60,13,sf::Color(180,185,200));
    drawHpBar(win,x+12,y+76,w-24,8,p.getHp()>0?(float)p.getHp()/100.f:0.f,
              sf::Color(35,38,60),sf::Color(60,200,100));
    string badge="พร้อม"; sf::Color bc2=sf::Color(30,130,80);
    if(!p.isAlive()){badge="DEAD";bc2=sf::Color(120,30,30);}
    else if(!p.canMove()){badge="STUN";bc2=sf::Color(180,120,20);}
    else if(p.isPoisoned()){badge="POISON";bc2=sf::Color(50,180,50);}
    sf::RectangleShape br(sf::Vector2f(80,22)); br.setPosition({x+12,y+92});
    br.setFillColor(sf::Color(bc2.r/3,bc2.g/3,bc2.b/3,200));
    br.setOutlineColor(bc2); br.setOutlineThickness(1.f); win.draw(br);
    drawText(win,font,badge,x+14,y+92,14,bc2);
}
 
void drawTileOverlay(sf::RenderWindow& win,sf::Font& font,
                     const string& line1,const string& line2,
                     float cx,float cy,float cs,sf::Color textCol){
    float bw=cs-6.f, bh=line2.empty()?14.f:26.f;
    sf::RectangleShape bg(sf::Vector2f(bw,bh));
    bg.setPosition({cx-bw/2.f,cy-bh/2.f});
    bg.setFillColor(sf::Color(0,0,0,165)); win.draw(bg);
    sf::Text t1(font,sf::String::fromUtf8(line1.begin(),line1.end()),10);
    t1.setFillColor(textCol);
    auto b1=t1.getLocalBounds();
    t1.setPosition({cx-b1.size.x/2.f,line2.empty()?cy-b1.size.y/2.f-1.f:cy-13.f});
    win.draw(t1);
    if(!line2.empty()){
        sf::Text t2(font,sf::String::fromUtf8(line2.begin(),line2.end()),10);
        t2.setFillColor(sf::Color(255,255,180));
        auto b2=t2.getLocalBounds();
        t2.setPosition({cx-b2.size.x/2.f,cy+1.f}); win.draw(t2);
    }
}
 
// ════════════════════════════════════════════════════════════════════
void drawBoard(sf::RenderWindow& win,sf::Font& font,GameEngine& game,
               float bx,float by,float bsize,bool showOverlay){
    float cs=bsize/10.f;
    auto& board=game.getBoard();
    auto& players=game.getPlayers();
    auto& gasZones=game.getGasZones();
 
    sf::RectangleShape bbg(sf::Vector2f(bsize+4,bsize+4));
    bbg.setPosition({bx-2,by-2}); bbg.setFillColor(sf::Color(15,18,30));
    bbg.setOutlineColor(sf::Color(60,65,100)); bbg.setOutlineThickness(2.f); win.draw(bbg);
 
    for(int cy=0;cy<10;cy++) for(int cx=0;cx<10;cx++){
        float px=bx+cx*cs, py=by+cy*cs;
        int type=board.getTileType(cx,cy);
        Tile* tileObj=board.getTileObj(cx,cy);
 
        // ── สีพื้น tile ──────────────────────────────────────────────
        sf::Color cellColor;
        switch(type){
            case 1: cellColor=sf::Color(55,30,30); break;   // wall
            // ── แก้ไข: ซ่อน tile ที่ไม่ใช่ wall เมื่อ showOverlay=false ──
            case 2: cellColor=showOverlay?sf::Color(30,55,30):sf::Color(35,42,65); break;  // trap
            case 3: cellColor=showOverlay?sf::Color(30,50,60):sf::Color(35,42,65); break;  // antidote
            case 4: cellColor=showOverlay?sf::Color(55,50,30):sf::Color(35,42,65); break;  // ?card
            default: cellColor=sf::Color(35,42,65);
        }
        bool inGas=false;
        for(auto& gz:gasZones)
            if(cx>=gz.first&&cx<=gz.first+2&&cy>=gz.second&&cy<=gz.second+2)
            { cellColor=sf::Color(60,35,20); inGas=true; }
 
        sf::RectangleShape cell(sf::Vector2f(cs-2,cs-2));
        cell.setPosition({px+1,py+1}); cell.setFillColor(cellColor); win.draw(cell);
 
        // ── แก้ไข: วาด tile icons เฉพาะตอน showOverlay=true ────────
        if(showOverlay){
            if(type==2){
                sf::RectangleShape m(sf::Vector2f(cs*.35f,cs*.35f));
                m.setPosition({px+cs*.3f,py+cs*.3f}); m.setFillColor(sf::Color(220,120,50,200)); win.draw(m);
            } else if(type==3){
                sf::RectangleShape h(sf::Vector2f(cs*.5f,cs*.15f));
                h.setPosition({px+cs*.25f,py+cs*.425f}); h.setFillColor(sf::Color(60,220,120,200)); win.draw(h);
                sf::RectangleShape v(sf::Vector2f(cs*.15f,cs*.5f));
                v.setPosition({px+cs*.425f,py+cs*.25f}); v.setFillColor(sf::Color(60,220,120,200)); win.draw(v);
            } else if(type==4){
                sf::RectangleShape m(sf::Vector2f(cs*.35f,cs*.35f));
                m.setPosition({px+cs*.3f,py+cs*.3f}); m.setFillColor(sf::Color(220,200,50,180)); win.draw(m);
            }
        }
 
        // Wall วาดเสมอ (ไม่ว่า showOverlay จะเป็นอะไร)
        if(type==1){
            sf::RectangleShape w(sf::Vector2f(cs-4,cs-4));
            w.setPosition({px+2,py+2}); w.setFillColor(sf::Color(70,35,35)); win.draw(w);
        }
 
        // grid lines
        sf::RectangleShape lh(sf::Vector2f(cs,1)); lh.setPosition({px,py}); lh.setFillColor(sf::Color(50,55,80,100)); win.draw(lh);
        sf::RectangleShape lv(sf::Vector2f(1,cs)); lv.setPosition({px,py}); lv.setFillColor(sf::Color(50,55,80,100)); win.draw(lv);
 
        float ccx=px+cs/2.f, ccy=py+cs/2.f;
 
        if(showOverlay && tileObj){
            sf::Color bc=tileObj->borderColor();
            if(bc.a>0){
                sf::RectangleShape bdr(sf::Vector2f(cs-2,cs-2));
                bdr.setPosition({px+1,py+1}); bdr.setFillColor(sf::Color(0,0,0,0));
                bdr.setOutlineColor(bc); bdr.setOutlineThickness(2.5f); win.draw(bdr);
            }
            if(inGas){
                sf::RectangleShape bdr(sf::Vector2f(cs-2,cs-2));
                bdr.setPosition({px+1,py+1}); bdr.setFillColor(sf::Color(0,0,0,0));
                bdr.setOutlineColor(sf::Color(255,160,30,200)); bdr.setOutlineThickness(2.5f); win.draw(bdr);
            }
            if(!tileObj->overlayLine1().empty())
                drawTileOverlay(win,font,tileObj->overlayLine1(),tileObj->overlayLine2(),
                                ccx,ccy,cs,tileObj->overlayColor());
            else if(inGas)
                drawTileOverlay(win,font,"GAS","-5 HP",ccx,ccy,cs,sf::Color(255,180,60));
        } else {
            // ── โหมดปกติ: แสดงเฉพาะ Wall ────────────────────────────
            if(type==1){
                sf::RectangleShape bdr(sf::Vector2f(cs-2,cs-2));
                bdr.setPosition({px+1,py+1}); bdr.setFillColor(sf::Color(0,0,0,0));
                bdr.setOutlineColor(sf::Color(180,60,60,200)); bdr.setOutlineThickness(2.f); win.draw(bdr);
                drawTileOverlay(win,font,"WALL","",ccx,ccy,cs,sf::Color(220,120,120));
            }
            // Gas zone outline แสดงเสมอแม้ไม่ showOverlay
            if(inGas){
                sf::RectangleShape bdr(sf::Vector2f(cs-2,cs-2));
                bdr.setPosition({px+1,py+1}); bdr.setFillColor(sf::Color(0,0,0,0));
                bdr.setOutlineColor(sf::Color(200,120,30,140)); bdr.setOutlineThickness(1.5f); win.draw(bdr);
            }
        }
    }
 
    // gas zone radius outline
    for(auto& gz:gasZones){
        float gx=bx+gz.first*cs, gy=by+gz.second*cs;
        sf::RectangleShape gr(sf::Vector2f(cs*3-2,cs*3-2));
        gr.setPosition({gx+1,gy+1}); gr.setFillColor(sf::Color(0,0,0,0));
        gr.setOutlineColor(showOverlay?sf::Color(255,160,30,240):sf::Color(200,120,30,140));
        gr.setOutlineThickness(showOverlay?3.f:1.5f); win.draw(gr);
        if(showOverlay)
            drawTileOverlay(win,font,"GAS ZONE","-5HP each",
                            gx+cs*1.5f,gy+cs*1.5f,cs*2.f,sf::Color(255,200,80));
    }
 
    // floating effect popups
    for(auto& pop:game.getPopups()){
        float px2=bx+pop.tile.x*cs+cs/2.f;
        float py2=by+pop.tile.y*cs+cs/2.f + pop.offsetY - cs*0.4f;
        int a = static_cast<int>(max(0.f,min(255.f,pop.alpha)));
 
        sf::Text txt(font,sf::String::fromUtf8(pop.label.begin(),pop.label.end()),11);
        txt.setFillColor(sf::Color(pop.color.r,pop.color.g,pop.color.b, static_cast<uint8_t>(a)));
        auto tb=txt.getLocalBounds();
        float bw=tb.size.x+10.f, bh=tb.size.y+6.f;
        sf::RectangleShape pill(sf::Vector2f(bw,bh));
        pill.setPosition({px2-bw/2.f,py2-2.f});
        pill.setFillColor(sf::Color(0,0,0,static_cast<uint8_t>(a*0.7f)));
        pill.setOutlineColor(sf::Color(pop.color.r,pop.color.g,pop.color.b, static_cast<uint8_t>(a)));
        pill.setOutlineThickness(1.f); win.draw(pill);
        txt.setPosition({px2-tb.size.x/2.f,py2}); win.draw(txt);
    }
 
    // players
    int ci=game.getCurrentPlayer();
    for(int i=0;i<4;i++){
        auto& p=players[i]; if(!p.isAlive()) continue;
        float px2=bx+p.getPosition().x*cs, py2=by+p.getPosition().y*cs;
        bool isCur=(i==ci&&!game.isGameOver());
        if(isCur){
            sf::RectangleShape hl(sf::Vector2f(cs-2,cs-2)); hl.setPosition({px2+1,py2+1});
            hl.setFillColor(sf::Color(0,0,0,0)); hl.setOutlineThickness(2.5f);
            hl.setOutlineColor(sf::Color(255,210,50)); win.draw(hl);
        }
        float r=cs*.32f;
        sf::CircleShape circle(r);
        circle.setPosition({px2+cs/2-r,py2+cs/2-r});
        circle.setFillColor(sf::Color(p.getColor().r,p.getColor().g,p.getColor().b,220));
        circle.setOutlineThickness(isCur?2.f:1.f);
        circle.setOutlineColor(isCur?sf::Color(255,210,50):sf::Color(200,200,200,150));
        win.draw(circle);
        sf::Text icon(font,string(1,p.getIcon()),(unsigned)(cs*.38f));
        icon.setFillColor(sf::Color(10,12,25)); icon.setStyle(sf::Text::Bold);
        auto bd=icon.getLocalBounds();
        icon.setPosition({px2+cs/2-bd.size.x/2,py2+cs/2-bd.size.y/2-2}); win.draw(icon);
    }
}
 
void drawLegend(sf::RenderWindow& win,sf::Font& font,float x,float y,float w,float h){
    drawRoundedRect(win,x,y,w,h,sf::Color(22,25,42,230),sf::Color(60,65,100),1.f);
    drawText(win,font,"คำอธิบาย",x+14,y+8,15,sf::Color(180,185,210));
    struct LI { sf::Color c; string t; };
    vector<LI> items={
        {sf::Color(200,90,50),"Trap: -10HP/Psn3T/+5HP"},
        {sf::Color(60,200,120),"Antidote: Cure+Teleport"},
        {sf::Color(220,200,50),"? Card: สุ่ม 7 ผล"},
        {sf::Color(120,90,90),"Wall: เดินผ่านไม่ได้"},
        {sf::Color(200,120,50),"Gas: -5 HP"},
    };
    float iy=y+28;
    for(auto& it:items){
        sf::RectangleShape dot(sf::Vector2f(8,8)); dot.setPosition({x+14,iy+3}); dot.setFillColor(it.c); win.draw(dot);
        drawText(win,font,it.t,x+28,iy,13,sf::Color(200,205,220)); iy+=22;
    }
    iy+=4;
    drawText(win,font,"ไพ่ดี",x+14,iy,13,sf::Color(80,220,120)); iy+=18;
    vector<pair<sf::Color,string>> cards={
        {sf::Color(200,80,80),"1:LifeSteal +5/คน"},
        {sf::Color(60,200,100),"2:ฮีล +10 HP"},
        {sf::Color(80,200,80),"3:พิษทุกคน 3T"},
        {sf::Color(100,150,220),"4:บล็อกทุกคน"},
    };
    for(auto& c:cards){
        sf::CircleShape dot(3.f); dot.setPosition({x+14,iy+4}); dot.setFillColor(c.first); win.draw(dot);
        drawText(win,font,c.second,x+26,iy,12,sf::Color(180,185,200)); iy+=18;
    }
    drawText(win,font,"ไพ่แย่",x+14,iy,13,sf::Color(220,80,80)); iy+=18;
    vector<pair<sf::Color,string>> bad={
        {sf::Color(220,80,80),"5:-10 HP ตัวเอง"},
        {sf::Color(150,80,180),"6:Drain ตัวเอง"},
        {sf::Color(220,160,50),"7:บล็อกตัวเอง"},
    };
    for(auto& c:bad){
        sf::CircleShape dot(3.f); dot.setPosition({x+14,iy+4}); dot.setFillColor(c.first); win.draw(dot);
        drawText(win,font,c.second,x+26,iy,12,sf::Color(180,185,200)); iy+=18;
    }
}
 
void drawLog(sf::RenderWindow& win,sf::Font& font,GameEngine& game,
             float x,float y,float w,float h){
    drawRoundedRect(win,x,y,w,h,sf::Color(18,20,38,230),sf::Color(60,65,100),1.f);
    drawText(win,font,"LOG",x+14,y+6,15,sf::Color(180,185,210));
    float ly=y+26;
    for(const auto& line:game.getLog()){
        drawText(win,font,line,x+10,ly,11,sf::Color(190,195,215));
        ly+=16; if(ly>y+h-12) break;
    }
}
 
void drawControls(sf::RenderWindow& win,sf::Font& font,GameEngine& game,
                  float x,float y,float w,float h,bool showOverlay){
    drawRoundedRect(win,x,y,w,h,sf::Color(20,22,40,200),sf::Color(55,60,95),1.f);
    auto& players=game.getPlayers(); int ci=game.getCurrentPlayer();
 
    if(!game.isGameOver()){
        sf::Color ic=showOverlay?sf::Color(80,200,120):sf::Color(100,100,130);
        sf::RectangleShape ind(sf::Vector2f(130,20)); ind.setPosition({x+w-148,y+6});
        ind.setFillColor(showOverlay?sf::Color(20,60,35):sf::Color(25,25,45));
        ind.setOutlineColor(ic); ind.setOutlineThickness(1.f); win.draw(ind);
        drawText(win,font,showOverlay?"[E] Effect: ON":"[E] Effect: OFF",x+w-145,y+7,12,ic);
 
        struct TL{ sf::Color c; string l; };
        vector<TL> tiles={{sf::Color(80,80,90),"ปกติ"},{sf::Color(200,80,40),"กับดัก"},
                          {sf::Color(60,200,100),"ยาแก้พิษ"},{sf::Color(80,200,80),"ไพ่?"},
                          {sf::Color(100,80,80),"กำแพง"}};
        float lx=x+16;
        for(auto& tl:tiles){
            sf::RectangleShape dot(sf::Vector2f(10,10)); dot.setPosition({lx,y+10}); dot.setFillColor(tl.c); win.draw(dot);
            drawText(win,font,tl.l,lx+13,y+7,13,sf::Color(190,195,215)); lx+=90;
        }
 
        drawText(win,font,"เทิร์นของ "+players[ci].getName()+"   หยอดลูกเต๋า",
                 x+w/2-130,y+32,16,sf::Color(240,240,100));
 
        float bx2=x+w/2-150,by2=y+54,bw=300,bh=30;
        sf::RectangleShape btn(sf::Vector2f(bw,bh)); btn.setPosition({bx2,by2});
        bool canRoll=!game.hasRolledYet();
        btn.setFillColor(canRoll?sf::Color(80,70,180):sf::Color(40,40,80));
        btn.setOutlineColor(canRoll?sf::Color(140,130,255):sf::Color(70,70,110));
        btn.setOutlineThickness(1.5f); win.draw(btn);
        string bl=canRoll?"หยอดลูกเต๋า [SPACE]":"Steps: "+to_string(game.getStepsLeft())+" [WASD]";
        drawText(win,font,bl,bx2+12,by2+7,14,sf::Color(220,220,255));
 
        drawText(win,font,"WASD/Arrow=เดิน  R=รีเซ็ต  E=Effect overlay",
                 x+w/2-190,y+92,14,sf::Color(150,155,180));
    } else {
        drawText(win,font,"GAME OVER!",x+w/2-80,y+10,22,sf::Color(255,210,50));
        drawText(win,font,game.getWinner(),x+w/2-100,y+40,18,sf::Color(100,220,120));
        drawText(win,font,"Press R to reset",x+w/2-80,y+68,14,sf::Color(180,185,210));
    }
}
 
// ════════════════════════════════════════════════════════════════════
int main(){
    srand(time(0));
    sf::RenderWindow window(sf::VideoMode({1400,820}),"Board Battle",sf::Style::Close);
    window.setFramerateLimit(60);
 
    sf::Font font;
    if(!font.openFromFile("C:/Windows/Fonts/tahoma.ttf"))
        if(!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
            if(!font.openFromFile("C:/Windows/Fonts/consola.ttf"))
                { cerr<<"Font error"<<endl; return 1; }
 
    GameEngine game;
    bool showEffectOverlay=false;
    sf::Clock clock;
 
    const float W=1400,H=820;
    const float cardW=230,cardH=130;
    const float boardLeft=cardW+30, boardRight=W-cardW-30;
    const float boardSize=min(boardRight-boardLeft-20.f,H-170.f);
    const float boardX=boardLeft+(boardRight-boardLeft-boardSize)/2;
    const float boardY=50.f;
    const float rightPanelX=boardX+boardSize+16;
    const float ctrlY=boardY+boardSize+10;
    const float ctrlH=H-ctrlY-10;
 
    while(window.isOpen()){
        float dt=clock.restart().asSeconds();
        game.tickPopups(dt);
 
        while (std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Space && !game.isGameOver() && !game.hasRolledYet())
                    game.rollDice();
                if (keyPressed->code == sf::Keyboard::Key::R)
                    game.resetGame();
                if (keyPressed->code == sf::Keyboard::Key::E)
                    showEffectOverlay = !showEffectOverlay;
 
                if (!game.isGameOver() && game.hasRolledYet() && game.getStepsLeft() > 0) {
                    string dir;
                    if (keyPressed->code == sf::Keyboard::Key::W || keyPressed->code == sf::Keyboard::Key::Up)    dir = "W";
                    if (keyPressed->code == sf::Keyboard::Key::S || keyPressed->code == sf::Keyboard::Key::Down)  dir = "S";
                    if (keyPressed->code == sf::Keyboard::Key::A || keyPressed->code == sf::Keyboard::Key::Left)  dir = "A";
                    if (keyPressed->code == sf::Keyboard::Key::D || keyPressed->code == sf::Keyboard::Key::Right) dir = "D";
                    if (!dir.empty()) game.movePlayer(dir);
                }
            }
        }
 
        window.clear(sf::Color(12,14,28));
 
        sf::RectangleShape tb(sf::Vector2f(W,40)); tb.setPosition({0,0});
        tb.setFillColor(sf::Color(18,20,40)); window.draw(tb);
        drawText(window,font,"Board Battle",16,9,18,sf::Color(200,205,230));
 
        auto& players=game.getPlayers(); int ci=game.getCurrentPlayer();
        float cy0=50,cy1=50+cardH+12;
        drawPlayerCard(window,font,players[0],14,cy0,cardW,cardH,ci==0&&!game.isGameOver());
        drawPlayerCard(window,font,players[1],14,cy1,cardW,cardH,ci==1&&!game.isGameOver());
        drawPlayerCard(window,font,players[2],rightPanelX,cy0,cardW,cardH,ci==2&&!game.isGameOver());
        drawPlayerCard(window,font,players[3],rightPanelX,cy1,cardW,cardH,ci==3&&!game.isGameOver());
 
        float legendY=cy1+cardH+12;
        float legendH=boardY+boardSize-legendY;
        if(legendH>60) drawLegend(window,font,rightPanelX,legendY,cardW,legendH);
        float logY=legendY+legendH+8, logH=H-logY-10;
        if(logH>40) drawLog(window,font,game,rightPanelX,logY,cardW,logH);
 
        drawBoard(window,font,game,boardX,boardY,boardSize,showEffectOverlay);
        drawControls(window,font,game,14,ctrlY,rightPanelX-18,ctrlH,showEffectOverlay);
 
        window.display();
    }
    return 0;
}
