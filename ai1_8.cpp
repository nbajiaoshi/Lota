#include "sdk.h"
#include<ctime>
#include<iostream>
#include<stdio.h>
#include<fstream>
#define ReinforceDistance 900
#define DefendTowerDistance 1600
#define Maxint 2100000000
#define W *10000
#define RainyValue 12500
#define MinHpValue 5
#define ArcherStepValue 1000
#define FarAwayValue 100
#define AvoidArrowRainSwitch true
#define MaxDetectPosSize 6
using namespace std;
int PushingLevel = 24;
int TowerAttackValue = 12500;
string LvUp[5][10] = {
"ARLU", "ARLU", "ARLU", "PALU", "PALU", "PALU",  "YellowPoint", "YellowPoint", "YellowPoint", "YellowPoint",
"PALU", "PALU", "PALU", "ARLU", "ARLU", "ARLU",  "YellowPoint", "YellowPoint", "YellowPoint", "YellowPoint",
"ARLU", "ARLU", "ARLU", "PALU", "PALU", "PALU",  "YellowPoint", "YellowPoint", "YellowPoint", "YellowPoint",
"PALU", "PALU", "PALU", "ARLU", "ARLU", "ARLU",  "YellowPoint", "YellowPoint", "YellowPoint", "YellowPoint",
"ARLU", "ARLU", "ARLU", "PALU", "PALU", "PALU",  "YellowPoint", "YellowPoint", "YellowPoint", "YellowPoint"};
const Pos PushingDetectPos[2][2][MaxDetectPosSize]={
Pos(102,35),Pos(56,8),Pos(58,28),Pos(90,42),Pos(97,25),Pos(67,8),
Pos(34,45),Pos(22,49),Pos(48,50),Pos(45,61),Pos(54,75),Pos(-1,-1),
Pos(48,115),Pos(94,142),Pos(92,122),Pos(60,108),Pos(53,125),Pos(83,142),
Pos(116,105),Pos(128,101),Pos(102,100),Pos(105,89),Pos(96,75),Pos(-1,-1)
};
int UpLevel[10]={100,225,375,550,750,975,1225,1500,1800,2125};
const Pos EMPTYPOS = Pos(), DEFAULFPOS = Pos(75, 75);
const int EMPTYID = 0, HIGHLEVEL = 0, MAXLEVEL = 10, MAXID = 100;
static int fixId[MAXID];
static Pos fixPos[MAXID];
int TowerBlockRange;
static bool towerDes[2],OurTowerDes[2];
typedef vector<const PUnit*> PUnits;
typedef vector<const PArea*> PAreas;
int DragonARRounds = 0;
int reviving[50];
int StimulateHp[50];
int EnemyHeroType[5];
int EnemyHeroTypeNum[3];
int RunAfterEnemyRounds[5] = {0};
bool RunAfterEnemyJudge[5];
bool LastRoundWantPush = false;
Pos StimulatePos[50];
Pos OurHeroPos[5];
Pos MonsterBornPos[50];
int EnemyExp[5]= {0};
int EnemyLevel[5] = {0};
double OurHeroInDizzy[5];
bool HeroHadBeenProtected[5];
int LastRoundsGetInfo[50];
PUnits OurHero,ours, others, nearBy,EnemyHero;
bool EnemyCanEscape[5];
bool EnemyIntheRain[5];
bool OurHeroIntheRain[5];
PAreas OurArea,EnemyArea;
const Pos* EnemyTower,*FriendlyTower;
const double EPS = 1e-7;
static vector<Pos> basic_blocks;
static vector<Pos> blocks;
static vector<Pos> blocks_without_Tower;
static vector<Pos> blocks_without_Tower_Rainy;
static vector<Pos> blocks_without_Rainy;
const PMap* MAP;
const PPlayerInfo* INFO;
Pos SpringPos;
bool Pushed[5] = {0};
int unitEvaluation(string name);
double StrengthCaculate(const Pos &centre, int range, int camp);
int Skill(string skillname);
bool ACanSeenPos(const PUnit* ptr,Pos& p){
    return (dis2(ptr->pos,p) <= ptr->view && MAP->height[ptr->pos.x][ptr->pos.y] + 2 > MAP->height[p.x][p.y]);
}
int MayHaveDamage(const PUnit* myhero){
    int hp = 0;
    if (myhero->findBuff("Rainy"))
        hp += Rainy_damage[myhero->findBuff("Rainy")->level];
    for (int i = 0;i < EnemyHero.size();i++){
        const PUnit* Enemy = EnemyHero[i];
        if (Enemy->canUseSkill(Skill("Spin")) && dis2(Enemy->pos,myhero->pos) <= Spin_range){
            hp += Spin_damage[Enemy->findSkill("Spin")->level];
            continue;
        }
        if (Enemy->canUseSkill(Skill("PiercingArrow")) && dis2(Enemy->pos,myhero->pos) <= PiercingArrow_range){
            hp += PiercingArrow_damage[Enemy->findSkill("PiercingArrow")->level];
            continue;
        }
        if (Enemy->canUseSkill(Skill("FlyingBlade")) && dis2(Enemy->pos,myhero->pos) <= FlyingBlade_range){
            hp += FlightBlade_damage[Enemy->findSkill("FlyingBlade")->level];
            continue;
        }
        if (Enemy->canUseSkill(Skill("Attack")) && dis2(Enemy->pos,myhero->pos) <= Enemy->range)
            hp += (Enemy->atk - myhero->def);
    }
    return hp;
}

void GetBlocks(){    
    blocks.clear();
    blocks_without_Tower.clear();
    blocks_without_Rainy.clear();
    blocks_without_Tower_Rainy.clear();
    for (int i=0; i<INFO->units.size(); ++i){
        basic_blocks.push_back(INFO->units[i].pos);
        blocks.push_back(INFO->units[i].pos);
        blocks_without_Tower.push_back(INFO->units[i].pos);
        blocks_without_Rainy.push_back(INFO->units[i].pos);
        blocks_without_Tower_Rainy.push_back(INFO->units[i].pos);
    }
    for (int i = 0; i < 2; ++i) if (!towerDes[i]){
        blocks.push_back(EnemyTower[i]);
        for (int x = -12;x <= 12;x++)
            for (int y = -12;y <= 12;y++)
                if (x*x+y*y<=TowerBlockRange){
                    blocks.push_back(EnemyTower[i] + Pos(x,y));
                    blocks_without_Rainy.push_back(EnemyTower[i] + Pos(x,y));
                }
    }
    for (int i = 0; i < EnemyArea.size();i++)
        for (int x = -5;x <= 5;x++)
            for (int y = -5;y <= 5;y++)
            if (x*x+y*y<=25){
                blocks.push_back(EnemyArea[i]->center + Pos(x,y));
                blocks_without_Tower.push_back(EnemyArea[i]->center + Pos(x,y));
            }

}
ostream& operator <<(ostream& O,Pos p){
    O << "(" << p.x << ',' << p.y << ")\n";
    return O;
}
bool operator <(const Pos& a,const Pos b){
    return a.y < b.y;
}
double MIN(double a,double b){
    if (a < b)
        return a;
    else
        return b;
}
double MAX(double a,double b){
    if (a > b)
        return a;
    else
        return b;
}
bool DetectBeforePushing(){
    return false;
}
int NeedToDefence(){
    if (EnemyHeroTypeNum[2] >= 3 && (OurTowerDes[1] || OurTowerDes[0]))
        return 2;
    if (EnemyHeroTypeNum[2] >= 1 && (OurTowerDes[1] || OurTowerDes[0]))
        return 1;
    if ((OurTowerDes[1] || OurTowerDes[0]) && !(towerDes[1] || towerDes[0]))
        return 2;
    return 0;
}
int Skill(string skillname);
int StrongPushingStimulate(PUnits& PushHero,int RLimit = 2){
    int Hp[5],TowerHp,damage[5],maxRainLevel[5] = {-1,-1,-1,-1,-1},Attack_damge[5] = {0},maxPiercingArrow[5] = {-1,-1,-1,-1,-1};
    for (int i  = 0;i < PushHero.size();i++)
        Hp[i] = PushHero[i]->hp;
    for (int i = 0; i < PushHero.size();i++)
        damage[i] = PushHero[i]->atk - 15;
    for (int j = 0;j < PushHero.size();j++){
        for (int i = 0;i < EnemyHero.size();i++){
            if (dis2(EnemyHero[i]->pos,PushHero[j]->pos) <= 625 && EnemyHero[i]->canUseSkill(Skill("ArrowsRain"))
                    && EnemyHero[i]->findSkill("ArrowsRain")->level > maxRainLevel[j])
                maxRainLevel[j] = EnemyHero[i]->findSkill("ArrowsRain")->level;
            if (dis2(EnemyHero[i]->pos,PushHero[j]->pos) <= 625 && EnemyHero[i]->canUseSkill(Skill("PiercingArrow"))
                    && EnemyHero[i]->findSkill("PiercingArrow")->level > maxRainLevel[j])
                maxPiercingArrow[j] = EnemyHero[i]->findSkill("PiercingArrow")->level;
            if (dis2(EnemyHero[i]->pos,PushHero[j]->pos) <= 200 &&
                    (EnemyHero[i]->canUseSkill(Skill("Yell") || EnemyHero[i]->canUseSkill(Skill("Charge")))))
                Hp[j] = 0;
            if (dis2(EnemyHero[i]->pos,PushHero[j]->pos) <= EnemyHero[i]->range)
                Attack_damge[i] += EnemyHero[i]->atk - PushHero[j]->def;
        }

    }
    for (int i = 0;i < PushHero.size();i++)
        Hp[i] = Hp[i] - PiercingArrow_damage[maxPiercingArrow[i]] > 0?Hp[i] - PiercingArrow_damage[maxPiercingArrow[i]]:0;
    int dam = 0;
    for (int i  = 0;i < PushHero.size();i++)
        dam += damage[i] * int(MAX((Hp[i] / (15 * maxRainLevel[i] + 55 * (1 + 0.3 * PushHero.size()) / PushHero.size() + Attack_damge[i]/2) + 1)/2,RLimit));
    return dam;
}
//-------------------------------------------------------------------------------From Monster.cc
#define Maxid 100

int MonsterPlayer = 2;
static int ang[Maxid][Maxid] = {0};
void calAng()
{
    PUnits ourUnits;
    INFO->findUnitByPlayer(MonsterPlayer,ourUnits);
    for (int i = 0; i < ourUnits.size(); ++i)
    {
        const PArg* arg = (*ourUnits[i])["LastHit"];
        if (arg)
        {
            for(int j=0; j<Maxid && j<arg->val.size(); ++j)
                if (arg->val[j] >= INFO->round-1)
                    ang[ourUnits[i]->id][j] += 5;
        }
    }
    for (int i = 0; i < OurHero.size(); ++i){
        const PUnit* otherUnit = OurHero[i];
        const PUnit* Monster;
        for (int j = 16; j < 39; ++j){
            Monster = INFO->findUnitById(j);
            if (Monster){
                if (dis2(otherUnit->pos,Monster->pos) <= 8)
                    ang[j][otherUnit->id] += 1;
                if (dis2(otherUnit->pos,Monster->pos) > 144)
                    ang[j][otherUnit->id] = 0;
                if (Monster->findBuff("Reviving"))
                    ang[j][otherUnit->id] = 0;
            }
            else if (dis2(otherUnit->pos,MonsterBornPos[j]) > 144 || MAP->height[MonsterBornPos[j].x][MonsterBornPos[j].y] < MAP->height[otherUnit->pos.x][otherUnit->pos.y] + 2)
                    ang[j][otherUnit->id] = 0;
        }
    }
}
//-------------------------------------------------------------------------------------fromsdk.cc
class ShortPath{
    int vst[151][151];
    Pos Q[151 * 151 + 1], Q_prior[151][151];
    int countF;
public:
    ShortPath():countF(0){}
    void findShortestPath(const PMap &map, Pos start, Pos dest, const std::vector<Pos> &blocks, std::vector<Pos> &_path)
    {
        int _sz = blocks.size();

        ++countF;
        _path.clear();
        for (int i=0; i<_sz; ++i)
        {
            if (checkPos(blocks[i]))
                vst[blocks[i].x][blocks[i].y] = countF;
        }
        if (!checkPos(start) || !checkPos(dest))
        {
            _path.push_back(Pos());
            return;
        }
        vst[start.x][start.y] = vst[dest.x][dest.y] = countF - 1;

        Pos _0, _1;

        int dir_ord[4];
        Q[0] = start;
        vst[start.x][start.y] = countF;
        for (int front=0, rear=0; front<=rear; ++front) {
            _0 = Q[front];
            for (int i=0; i<4; ++i) dir_ord[i] = (i + front + countF) % 4;

            for (int i=0; i<4; ++i) {
                _1 = _0 + Pos(dir[dir_ord[i]][0], dir[dir_ord[i]][1]);
                if (checkPos(_1) && abs(map.height[_0.x][_0.y] - map.height[_1.x][_1.y]) <= 1 && vst[_1.x][_1.y] != countF) {
                    vst[_1.x][_1.y] = countF;
                    Q[++rear] = _1;
                    Q_prior[_1.x][_1.y] = _0;
                }
            }
        }
        if (vst[dest.x][dest.y] == countF) {
            for (_0 = dest; _0 != start; _0 = Q_prior[_0.x][_0.y])
                _path.push_back(_0);
            _path.push_back(_0);
            reverse(_path.begin(), _path.end());
        } else _path.push_back(Pos());
    }
}SelfShortPath;

//---------------------------------------------------------------------------------------------------
const PUnit* belongs(int id, PUnits& units) //???unit?????vector?§Ñ??????id??unit
{
    for (int i=0;i<units.size();++i)
        if (units[i]->id == id)
            return units[i];
    return NULL;
}
int Skill(string skillname){
    for (int i = 0; i < 20;i++)
        if (skillname == Skill_name[i])
            return i;
    return -1;
}

double StrengthCaculate(const Pos& centre,int range,int camp){
    double ans = 0;
    if (camp == INFO->camp){
        for (int i = 0;i < OurHero.size();i++)
            if (dis2(OurHero[i]->pos,centre) < range)
                ans += (3 + OurHero[i]->level);
    }
    else {
        for (int i = 0;i < EnemyHero.size();i++)
            if (dis2(EnemyHero[i]->pos,centre) < range)
                ans += (3 + EnemyHero[i]->level);
    }
    return ans;
}
double StrengthCompare(const Pos& centre,int range){
    double Our = 0,Enemy = 0;
    for (int i = 0;i < OurHero.size();i++)
        if (dis2(OurHero[i]->pos,centre) < range){
            Our += (3 + OurHero[i]->level) * (OurHero[i]->hp + OurHero[i]->max_hp / 2);
            if (OurHeroInDizzy[OurHero[i]->id % 5] > 0)
                return 1;
        }
    for (int i = 0;i < EnemyHero.size();i++)
        if (dis2(EnemyHero[i]->pos,centre) < range)
            Enemy += (3 + EnemyHero[i]->level) * (EnemyHero[i]->hp + EnemyHero[i]->max_hp / 2);
    if (Enemy == 0)
        return Maxint;
    return Our / Enemy;
}

int PushingTower = -1;
int unitEvaluation(string name)
{

    if (name == string("Fighter") || name == string("Assassin") || name == string("Archer"))
        return -1;
    if (PushingTower > 0){
        if (name == string("Tower"))
            return 0;
        else
            return Maxint / 10000;
    }
    if ((name == string("Tower") && PushingTower <= 0) || name == string("Roshan"))
        return Maxint / 10000;
    if (name == string("Wolf") || name == string("StoneMan") || name == string("Bow"))
        return 6;
    if (name == string("Dragon"))
        return 9;
    if (PushingTower <=0 || name == string("Tower"))
        return 0;
    else
        return Maxint / 10000;
}
void levelUp(const PUnit* a, PCommand &cmd)
{
    Operation op;
    op.id = a->id;
    op.type = LvUp[a->id%5][a->level + 1 - (*a)["Exp"]->val[3]];
    cmd.cmds.push_back(op);
}
void chooseHero(const PPlayerInfo &info, PCommand &cmd) //??0?????????
{
    for(int i=0;i<info.units.size();++i)
    {
        Operation op;
        int r=2;
        if(r==0)
            op.type="ChooseAssassin";
        else if(r==1)
            op.type="ChooseFighter";
        else if(r==2)
            op.type="ChooseArcher";
        op.targets.clear();
        op.id=info.units[i].id;
        cmd.cmds.push_back(op); //?????????
    }
}
//--------------------------------------------------------------------------------------------------------------value.
class Strategy;
class Advicer;
int TotalTask = 0,TotalAdvicer = 0;
Strategy* HeroTask[100];
Strategy* Task[100];
Advicer* Advicers[100];
//-----------------------------------------------------------------------------------------------------Factor.cc
class BFS;
enum Direct{
    Horizontal = 0,
    Vertical,
    None
};
class Factor{
public:
    virtual int evaluate(Pos pos) = 0;
};
class AvoidInFriendlyTower:public Factor{
public:
    virtual int evaluate(Pos pos);
};
class AvoidEnemyTower:public Factor{
protected:
    const PUnit* myhero;
public:
    AvoidEnemyTower(const PUnit* m):myhero(m){}
    virtual int evaluate(Pos pos);
};
class AvoidMeeleHero:public Factor{
public:
    virtual int evaluate(Pos pos);
};
class Dispersed:public Factor{
public:
    virtual int evaluate(Pos pos);
};
class ArcherEasyAttackEnemy:public Factor{
public:
    virtual int evaluate(Pos pos);
};
class AvoidArrowRain:public Factor{
protected:
    const PUnit* myhero;
public:
    AvoidArrowRain(const PUnit* m):myhero(m){}
public:
    virtual int evaluate(Pos pos);
};
class FarFromTarget:public Factor{
protected:
    int MoveValue;
    const Pos& target;
public:
    FarFromTarget(const Pos& p,int v =FarAwayValue):target(p),MoveValue(v){}
    virtual int evaluate(Pos pos);
};
class MoveToEvaluate:public Factor{
int MoveValue;
protected:
    const Pos& target;
public:
    MoveToEvaluate(const Pos& p,int v = FarAwayValue):target(p),MoveValue(v){}
    virtual int evaluate(Pos pos);
};
class FarAwayFromEnemy:public Factor{
public:
    virtual int evaluate(Pos pos);
};
class RunAfterEnemy:public Factor{
public:
    virtual int evaluate(Pos pos);
};
class FarAwayFromOtherMonster:public Factor{
public:
    virtual int evaluate(Pos pos);
};
class PullMonsterLimit:public Factor{
protected:
    int id;
public:
    PullMonsterLimit(int ii):id(ii){}
    virtual int evaluate(Pos pos);
};
class PullBowEvaluate:public Factor{
protected:
    double dist;
    const PUnit *PullBow,*myhero;
    const Pos& PullTower;

public:
    PullBowEvaluate(const PUnit* m,const PUnit* b,const Pos& t);
    virtual int evaluate(Pos pos);
};
class InAreaEvaluate:public Factor{
protected:
    int _r,_v;
    const Pos& _c;
public:
    InAreaEvaluate(int range,const Pos& center,int value):_r(range),_c(center),_v(value){}
    virtual int evaluate(Pos pos);
};
int InAreaEvaluate::evaluate(Pos pos){
    if (dis2(_c,pos) <= _r)
        return _v;
    else
        return 0;
}
PullBowEvaluate::PullBowEvaluate(const PUnit* m,const PUnit* b,const Pos& t):myhero(m),PullBow(b),PullTower(t){
    if (dis2(PullBow->pos,myhero->pos) <= PullBow->range)
        dist = 12.001;
    else
        dist = 14.001;
}
int PullBowEvaluate::evaluate(Pos pos){
    int A = PullBow->pos.y - PullTower.y;
    int B = PullTower.x - PullBow->pos.x;
    int C = -A * PullTower.x - B*PullTower.y;
    int valu = int(-500*fabs(pos.x * A + pos.y * B + C) / sqrt(A*A+B*B));
    if (dist > dis(pos,PullBow->pos))
        valu += int (200 * dis(pos,PullBow->pos));
    return valu;
}

int PullMonsterLimit::evaluate(Pos pos){
    if (dis2(MonsterBornPos[id],pos) > 324)
        return -2000;
    if (INFO->findUnitById(id) && dis2(INFO->findUnitById(id)->pos,pos) > 200)
        return -2000;
    return 0;
}
int FarAwayFromOtherMonster::evaluate(Pos pos){
    PUnits near;
    INFO->findUnitInArea(pos,8,near);
    int valu = 0;
    for (int i = 0 ; i < near.size();i++)
        if (near[i]->id >= 16 && near[i]->id <= 38)
            valu -= 1500;
    return valu;
}

int RunAfterEnemy::evaluate(Pos pos){
    double mindis = Maxint;
    for (int i = 0; i < EnemyHero.size();i++) if (dis(pos,EnemyHero[i]->pos) < mindis) mindis = dis(pos,EnemyHero[i]->pos);
    if (mindis > Maxint - 1)
        return 0;
    else
        return mindis < 3?0:-int((mindis - 3) * 1000);
}
int FarAwayFromEnemy::evaluate(Pos pos){
    int valu = 0;
    double range;
    for (int i = 0; i < EnemyHero.size();i++){
        range = MAX(sqrt(EnemyHero[i]->range) + sqrt(EnemyHero[i]->speed),9);
        valu -= 100*(dis(EnemyHero[i]->pos,pos) <= range?abs (range - dis(EnemyHero[i]->pos,pos)):0);
    }
    return valu;
}

int MoveToEvaluate::evaluate(Pos pos){
    return -int(dis(pos,target) * MoveValue);
}
int FarFromTarget::evaluate(Pos pos){
    return int(dis(pos,target) * MoveValue);
}
int AvoidInFriendlyTower::evaluate(Pos pos){
    bool MeleeEnemyAround = false;
    for (int i = 0; i < EnemyHero.size();i++)
        if (dis2(EnemyHero[i]->pos,pos) < 144 && EnemyHero[i]->range < 30)
            MeleeEnemyAround = true;
    int valu = 0;
    for (int i = 0;i < 2;i++)
        if (!OurTowerDes[i]  && MeleeEnemyAround && dis2(pos,FriendlyTower[i]) <= 51)
            valu += 3500;
        else
            if (!OurTowerDes[i] && dis2(pos,FriendlyTower[i]) <= 625)
                valu += int ((25 - dis(pos,FriendlyTower[i])) * 150);
    return valu;

}
int AvoidEnemyTower::evaluate(Pos pos){
    int Step = 0;
    for (int i = 0;i < 2;i++)
        if (!towerDes[i] && dis2(pos,EnemyTower[i]) <= 100){
            Step =int ((10 - dis(pos,EnemyTower[i])) / (sqrt(myhero->speed) - 0.5) + 1);
        }
    return -Step * TowerAttackValue;
}
int AvoidMeeleHero::evaluate(Pos pos){
    int valu = 0;
    for (int i = 0; i < EnemyHero.size();i++) if (EnemyHero[i]->range < 25){
        valu -= dis2(EnemyHero[i]->pos,pos) < EnemyHero[i]->range?2000:
     int(200*(dis2(EnemyHero[i]->pos,pos) < 47?9 - dis(EnemyHero[i]->pos,pos):0));
    }
    return valu;
}
int Dispersed::evaluate(Pos pos){
    int valu = 0;
    const PUnit* enemy;
    int Dispersed_Rate = 40;
    int MaxPiercingArrow = -1,MaxArrowsRain = -1,MaxYell = -1,MaxSpin = -1;
    for (int i = 0;i < EnemyHero.size();i++){
        enemy = EnemyHero[i];
        if (dis2(enemy->pos,pos) < 170 && enemy->canUseSkill(Skill("PiercingArrow")) && enemy->findSkill("PiercingArrow")->level >  MaxPiercingArrow)
            MaxPiercingArrow = enemy->findSkill("PiercingArrow")->level;
        if (dis2(enemy->pos,pos) < 100 && enemy->canUseSkill(Skill("ArrowsRain")) && enemy->findSkill("ArrowsRain")->level >  MaxPiercingArrow)
            MaxArrowsRain = enemy->findSkill("ArrowsRain")->level;
        if (dis2(enemy->pos,pos) < 58 && enemy->canUseSkill(Skill("Yell")) && enemy->findSkill("Yell")->level >  MaxYell)
            MaxYell= enemy->findSkill("Yell")->level;
        if (dis2(enemy->pos,pos) < 58 && enemy->canUseSkill(Skill("Spin")) && enemy->findSkill("Spin")->level >  MaxSpin)
            MaxSpin = enemy->findSkill("Spin")->level;
    }
    for (int i = 0; i < 5;i++){
        //valu -= dis2(OurHeroPos[i],pos) < 25?int(Dispersed_Rate*(5 - dis(OurHeroPos[i],pos))):0;
        valu -= dis2(OurHeroPos[i],pos) < 9?int((MaxPiercingArrow + 1)*100*(5 - dis(OurHeroPos[i],pos))):0;
        valu -= dis2(OurHeroPos[i],pos) < 25?int((MaxArrowsRain + 1)*200*(6 - dis(OurHeroPos[i],pos))):0;
        valu -= dis2(OurHeroPos[i],pos) < 13?int((MaxYell + 1)*50*(5 - dis(OurHeroPos[i],pos))):0;
        valu -= dis2(OurHeroPos[i],pos) < 13?int((MaxSpin + 1)*100*(5 - dis(OurHeroPos[i],pos))):0;
    }
    return valu;
}
int ArcherEasyAttackEnemy::evaluate(Pos pos){
    int TotalMinHp = Maxint,minhp = Maxint,rank = 0;
    for (int i = 0 ; i < EnemyHero.size();i++) if (StimulateHp[EnemyHero[i]->id] > 0 && dis2(EnemyHero[i]->pos,pos) <= 81 && minhp > StimulateHp[EnemyHero[i]->id]){
        //minhp = StimulateHp[EnemyHero[i]->id];
    }
    if (minhp == Maxint){
        double speed = 4.5;
        double range = 9;
        int valu = Maxint;
        for (int i = 0 ; i < EnemyHero.size();i++) if (valu > ArcherStepValue * (dis(pos,EnemyHero[i]->pos) - range) / speed + StimulateHp[EnemyHero[i]->id] * MinHpValue)
            valu = int(ArcherStepValue * (dis(pos,EnemyHero[i]->pos) - range) / speed + StimulateHp[EnemyHero[i]->id] * MinHpValue);
        return -valu;
    }
    else {
        return -minhp * MinHpValue;
    }
}
int AvoidArrowRain::evaluate(Pos pos){
    int nowRainyStatus = 1;
    if (!OurHeroIntheRain[myhero->id % 5])
        nowRainyStatus = 0;
    int RainyStep;
    for (int i =0;i <EnemyArea.size();i++)
        if (EnemyArea[i]->containPos(pos)) {
            RainyStep =int ((EnemyArea[i]->radius - dis(pos,EnemyArea[i]->center)) / (sqrt(myhero->speed) - 0.5) + 2);
            RainyStep = MIN(RainyStep,EnemyArea[i]->timeLeft);
            if (RainyStep > nowRainyStatus)
                nowRainyStatus =  RainyStep;
        }
    return -nowRainyStatus * RainyValue;
}
//--------------------------------------------------------------------------------------------TargetEstimate.cc
class TargetEstimate{  //????????
    protected:
        Pos BestTarget;
        int value;
        vector<Factor*> Factors;
        int myheroPosvalue;
        const PUnit* myhero;
    public:
        int PosEvaluate(Pos pos);
        TargetEstimate(const PUnit* m):value(-Maxint),myheroPosvalue(0),myhero(m){}
        int getvalue(){return value;}
        Pos gettarget(){return BestTarget;}
        virtual void compare(Pos pos,BFS* bfs);
};

TargetEstimate* TargetBFS(TargetEstimate* estimate,const PUnit* my_hero,int step,vector<Pos>& _path,vector<Pos>& block,int value_max = Maxint);
class FarAwayFromTarget:public TargetEstimate{
public:
    FarAwayFromTarget(const Pos& p,const PUnit* m):TargetEstimate(m){
        Factors.push_back(new AvoidArrowRain(myhero));
        Factors.push_back(new FarFromTarget(p));
        Factors.push_back(new AvoidInFriendlyTower);
        Factors.push_back(new FarAwayFromOtherMonster);
        Factors.push_back(new AvoidEnemyTower(m));
        myheroPosvalue = PosEvaluate(m->pos);
    }
};
class MoveToTarget:public TargetEstimate{
public:
    MoveToTarget(const Pos& p,const PUnit* m):TargetEstimate(m){
        Factors.push_back(new AvoidArrowRain(myhero));
        Factors.push_back(new MoveToEvaluate(p));
        Factors.push_back(new AvoidEnemyTower(m));
        Factors.push_back(new FarAwayFromOtherMonster);
        myheroPosvalue = PosEvaluate(m->pos);
    }
};
class FightingMoveEvaluation:public TargetEstimate{
public:
    FightingMoveEvaluation(const PUnit* m):TargetEstimate(m){
        Factors.push_back(new AvoidArrowRain(m));
        Factors.push_back(new AvoidInFriendlyTower);
        Factors.push_back(new AvoidMeeleHero);
        Factors.push_back(new Dispersed);
        Factors.push_back(new ArcherEasyAttackEnemy);
        Factors.push_back(new AvoidEnemyTower(m));
        Factors.push_back(new FarAwayFromOtherMonster);
        myheroPosvalue = PosEvaluate(m->pos);
    }
};
class RunAwayFromPowerfulEnemy:public TargetEstimate{
public:
    RunAwayFromPowerfulEnemy(const PUnit* m):TargetEstimate(m){
        Factors.push_back(new AvoidArrowRain(m));
        Factors.push_back(new AvoidInFriendlyTower);
        Factors.push_back(new MoveToEvaluate(SpringPos));
        Factors.push_back(new AvoidMeeleHero);
        Factors.push_back(new Dispersed);
        Factors.push_back(new AvoidEnemyTower(m));
        Factors.push_back(new FarAwayFromEnemy);
        Factors.push_back(new FarAwayFromOtherMonster);
        myheroPosvalue = PosEvaluate(m->pos);
    }
};
class RunAfterWeakEnemy:public TargetEstimate{
public:
    RunAfterWeakEnemy(const PUnit* m):TargetEstimate(m){
        Factors.push_back(new AvoidArrowRain(m));
        Factors.push_back(new AvoidMeeleHero);
        Factors.push_back(new Dispersed);
        Factors.push_back(new AvoidEnemyTower(m));
        Factors.push_back(new RunAfterEnemy);
        Factors.push_back(new FarAwayFromOtherMonster);
        myheroPosvalue = PosEvaluate(m->pos);
    }
};
class PushingTowerMove:public TargetEstimate{
public :
    PushingTowerMove(const PUnit* m):TargetEstimate(m){
        Factors.push_back(new AvoidArrowRain(m));
        Factors.push_back(new Dispersed);
        Factors.push_back(new AvoidEnemyTower(m));
        Factors.push_back(new AvoidMeeleHero);
        Factors.push_back(new FarAwayFromEnemy);
        myheroPosvalue = PosEvaluate(m->pos);
    }
    virtual void compare(Pos pos,BFS* bfs);
};
class PushingDispersed:public TargetEstimate{
public :
    PushingDispersed(const PUnit* m):TargetEstimate(m){
        Factors.push_back(new AvoidArrowRain(m));
        Factors.push_back(new AvoidEnemyTower(m));
        Factors.push_back(new Dispersed);
        Factors.push_back(new ArcherEasyAttackEnemy);
        Factors.push_back(new FarAwayFromOtherMonster);
        myheroPosvalue = PosEvaluate(m->pos);
    }
};
class MonsterPosStimulate:public TargetEstimate{
public:
    MonsterPosStimulate(const Pos& p,const PUnit* m):TargetEstimate(m){
        Factors.push_back(new MoveToEvaluate(p));
    }
};
class PullMonsterMove:public TargetEstimate{
public:
    PullMonsterMove(int TowerNum,int MonsterId,const PUnit* m,const Pos& MonsterPos):TargetEstimate(m){
        Factors.push_back(new AvoidArrowRain(myhero));
        Factors.push_back(new FarFromTarget(MonsterPos,FarAwayValue));
        Factors.push_back(new MoveToEvaluate(FriendlyTower[TowerNum],FarAwayValue));
        Factors.push_back(new InAreaEvaluate(100,FriendlyTower[TowerNum],2000));
        Factors.push_back(new InAreaEvaluate(13,MonsterPos,-2000));
        Factors.push_back(new PullMonsterLimit(MonsterId));
        myheroPosvalue = PosEvaluate(m->pos);
    }
};
class PullBowMove:public TargetEstimate{
public:
    PullBowMove(int TowerNum,int MonsterId,const PUnit* m):TargetEstimate(m){
        if (INFO->findUnitById(MonsterId))
            Factors.push_back(new PullBowEvaluate(m,INFO->findUnitById(MonsterId),FriendlyTower[TowerNum]));
        Factors.push_back(new AvoidArrowRain(myhero));
        Factors.push_back(new PullMonsterLimit(MonsterId));
        myheroPosvalue = PosEvaluate(m->pos);
    }
};
class DoNotWantMove:public TargetEstimate{
public:
    DoNotWantMove(const PUnit* m):TargetEstimate(m){
        Factors.push_back(new AvoidArrowRain(m));
        Factors.push_back(new AvoidEnemyTower(m));
        Factors.push_back(new MoveToEvaluate(m->pos));
        Factors.push_back(new FarAwayFromOtherMonster);
        myheroPosvalue = PosEvaluate(m->pos);
    }
};
void PushingTowerMove::compare(Pos pos,BFS* bfs){
    int valu = PosEvaluate(pos) - myheroPosvalue;
    if (dis2(pos,EnemyTower[0]) > 81 && dis2(pos,EnemyTower[1]) > 81) valu = -Maxint;
    if (valu >= value){
        value = valu;
        BestTarget = pos;
    }
}
int TargetEstimate::PosEvaluate(Pos pos){
    int valu  = 0;
    for (int i = 0;i < Factors.size();i++){
        valu += Factors[i]->evaluate(pos);
    }
    return valu;
}
void TargetEstimate::compare(Pos pos,BFS* bfs){
    int valu = PosEvaluate(pos) - myheroPosvalue;
    if (valu >= value){
        value = valu;
        BestTarget = pos;
    }
}
//-----------------------------------------------------------------------------------------------------------BFS.cc
struct BFS{
    int head,tail,step;
    const int size_max,centre_fix;
    void enqueue(int x,int y);
    Pos* queue;
    double* spd;
    Direct* dir;
    int *prev,*step_remain;
    bool stone[41][41];
    TargetEstimate* estimate;
    const PUnit* my_hero;
    BFS(TargetEstimate* est,const PUnit* my,int s,vector<Pos>& block);
    int FindBestTarget(vector<Pos>& _path,int = Maxint);
    ~BFS();
};
BFS::BFS(TargetEstimate* est,const PUnit* my,int s,vector<Pos>& block):
size_max(int(sqrt(my->speed) + 1) * s * 2 + 1),centre_fix(int(sqrt(my->speed) + 1) * s),
head(0),tail(0),estimate(est),my_hero(my),step(s)
{
    dir = new Direct[size_max * size_max];
    queue = new Pos[size_max * size_max];
    prev = new int[size_max * size_max];
    step_remain = new int[size_max * size_max];
    spd = new double[size_max * size_max];
    memset(stone,0,sizeof(stone));
    dir[0] = None;
    spd[0] = 0;
    step_remain[0] = step;
    queue[0].x = 0; queue[0].y = 0;
    for (int i = 0;i < block.size();i++){
        if (abs((block[i].x - my_hero->pos).x) < int(sqrt(my->speed) + 1) * s && (abs((block[i] - my_hero->pos).y)) < int(sqrt(my->speed) + 1) * s)
            stone[block[i].x - my_hero->pos.x + centre_fix][block[i].y - my_hero->pos.y + centre_fix] = true;
    }
    stone[centre_fix][centre_fix] = true;

}
BFS::~BFS(){
    delete dir;
    delete queue;
    delete prev;
    delete step_remain;
    delete spd;
}
void BFS::enqueue(int x,int y)
{
    Pos now = queue[head] + my_hero->pos + Pos(x,y);
    if (!checkPos(now) || abs(MAP->height[now.x][now.y] - MAP->height[now.x - x][now.y - y]) > 1) return;
    bool& st = stone[queue[head].x + centre_fix + x][queue[head].y + centre_fix + y];
    double speed_remain = spd[head];
    if (st) return;
    if (((dir[head] == Horizontal && x == 0) || (dir[head] == Vertical && y == 0)) && speed_remain > 0.414 - EPS)
    {
        speed_remain -= 0.414;
        queue[++tail] = queue[head] + Pos(x,y);
        dir[tail] = None;
        spd[tail] = speed_remain;
        step_remain[tail] = step_remain[head];
        prev[tail] = head;
        st = true;
    }
    else
    {
        speed_remain -= 1;
        if (speed_remain > 0 - EPS || step_remain[head] > 0){
            queue[++tail] = queue[head] + Pos(x,y);
            if (y == 0)
                dir[tail] = Horizontal;
            else
                dir[tail] = Vertical;
            if (speed_remain > 0 - EPS){
                spd[tail] = speed_remain;
                step_remain[tail] = step_remain[head];
            }
            else {
                spd[tail] = sqrt(my_hero->speed) - 1;
                step_remain[tail] = step_remain[head] - 1;
            }
            prev[tail] = head;
            st = true;
        }
    }

}
int BFS::FindBestTarget(vector<Pos>& _path,int value_max){
    _path.clear();
    while (head <= tail){
        estimate->compare(queue[head] + my_hero->pos,this);
        if (estimate->getvalue() > value_max) break;
        if (dir[head] == Horizontal){
            enqueue(0,1);
            enqueue(0,-1);
            enqueue(1,0);
            enqueue(-1,0);
        }
        else {
            enqueue(1,0);
            enqueue(-1,0);
            enqueue(0,1);
            enqueue(0,-1);
        }
        head++;
    }
    int best;
    for (int i  = 0; i <= tail;i++) if (queue[i] + my_hero->pos == estimate->gettarget()){
        best = i;
        break;
    }
    while (best != 0){
        _path.push_back(my_hero->pos + queue[best]);
        best = prev[best];
    }
    _path.push_back(my_hero->pos);
    reverse(_path.begin(), _path.end());
    return estimate->getvalue();
}
TargetEstimate* TargetBFS(TargetEstimate* estimate,const PUnit* my_hero,int step,vector<Pos>& _path,vector<Pos>& block,int value_max){
    if (AvoidArrowRainSwitch && !OurHeroIntheRain[my_hero->id % 5])
        for (int i = 0; i < EnemyArea.size();i++)
            for (int x = -5;x <= 5;x++)
                for (int y = -5;y <= 5;y++)
                    if (x*x+y*y<=25)
                        block.push_back(EnemyArea[i]->center + Pos(x,y));
    BFS bfs(estimate,my_hero,step,block);
    bfs.FindBestTarget(_path,value_max);
    if (AvoidArrowRainSwitch && !OurHeroIntheRain[my_hero->id % 5])
        for (int i = 0; i < EnemyArea.size();i++)
            for (int x = -5;x <= 5;x++)
                for (int y = -5;y <= 5;y++)
                    if (x*x+y*y<=25)
                        block.pop_back();
    return estimate;
}
//--------------------------------------------------------------------------------------------------------------strategy.cc
class Strategy{
	protected:        
		int _hero[5];
        const PUnit* myhero;
        int Taskid(){for (int i = 0; i < TotalTask;i++) if (this == Task[i]) return i; return -1;}
        bool FightMonster(PCommand &cmd,const PUnit* ptr);
        void fightingMonster(const PUnit* a, const PUnit* b, PCommand &cmd);
        void Move(const PUnit* myhero,PCommand &cmd,Pos target);
        void Move(const PUnit* myhero,PCommand &cmd,TargetEstimate* estimate,vector<Pos>& block);
        virtual int StepToReach(const PUnit* hero,Pos target);
        void ChangeTask(Strategy* T,PCommand &cmd);
	public:
        Strategy(string name = "Strategy"):TaskName(name),num(0),Max(5){}
        void clear(){while (num) erase(0);}
        virtual void command(PCommand &cmd) = 0;
        void append(const PUnit* myhero);
        void erase(const PUnit* myhero);
        void erase(int i);
        int Max,num;//**********************************
        vector<Pos> TaskPos;//**********************************
        const string TaskName;//******************************************
};
class FightMonsterStrategy:public Strategy{// ???--------------------------------------------------------------------------------------
    protected:
        int id[20];
		int TargetSize;
        int now;
        bool TargetCanNotAttack();        
	public:
        FightMonsterStrategy(string name = "FightMonsterStrategy"):Strategy(name){}
        FightMonsterStrategy(int m,int m1,int m2,int m3,int m4,string name = "FightMonsterStrategy");
        virtual void NextTarget();
        virtual void command(PCommand &cmd);
};
FightMonsterStrategy::FightMonsterStrategy(int m,int m1,int m2,int m3,int m4,string name):Strategy(name){
    Max = m;
    id[0] = m1;
    id[1] = m2;
    TargetSize = 2;
    if (m3){
        id[2] = m3;
        TargetSize = 3;
        if (m4){
            id[3] = m4;
            TargetSize = 4;
        }
    }
    if (INFO->camp)
        for (int i = 0; i < TargetSize;i++){
            if (id[i] >= 16 && id[i] < 24)
                id[i] = id[i] < 20?id[i] + 4 : id[i] - 4;
            if (id[i] >= 24 && id[i] < 30)
                id[i] = id[i] < 27?id[i] + 3 : id[i] - 3;
            if (id[i] >= 30 && id[i] < 34)
                id[i] = id[i] < 32?id[i] + 2 : id[i] - 2;
            if (id[i] >= 34 && id[i] < 36)
                id[i] = id[i] < 35?id[i] + 1 : id[i] - 1;
            if (id[i] >= 36 && id[i] < 38)
                id[i] = id[i] < 37?id[i] + 1 : id[i] - 1;
        }
    TaskPos.clear();
    for (int i = 0;i < TargetSize;i++)
        TaskPos.push_back(MonsterBornPos[id[i]]);
    now = 0;
}

class PullMonsterStrategy:public FightMonsterStrategy{
protected:
    int TowerNum;
    int PullBowSign;
public:
    PullMonsterStrategy(int n,string name = "PullMonsterStrategy"):TowerNum(n),FightMonsterStrategy(name),PullBowSign(0){}
    virtual void command(PCommand &cmd);
};
class PullMonsterStrategy_DogAndWolf:public PullMonsterStrategy{
public:
    PullMonsterStrategy_DogAndWolf(string name = "PullMonsterStrategy_DogAndWolf"):PullMonsterStrategy(1,name){
        Max = 1;
        id[0] = 30 + 2 - 2 * INFO->camp;
        id[1] = 16 + 7 - 4 * INFO->camp;
        id[2] = 16 + 3 + 4 * INFO->camp;
        TargetSize = 2;
        TaskPos.clear();
        for (int i = 0;i < TargetSize;i++)
            TaskPos.push_back(MonsterBornPos[id[i]]);
        now = 0;
    }
    virtual void command(PCommand &cmd);
};
class PullMonsterStrategy_26_22:public PullMonsterStrategy{
public:
    PullMonsterStrategy_26_22(string name = "PullMonsterStrategy_26_22"):PullMonsterStrategy(0,name){
        Max = 1;
        id[0] = 26 + 3 * INFO->camp;
        id[1] = 16 + 6 - 4 * INFO->camp;
        TargetSize = 2;
        TaskPos.clear();
        for (int i = 0;i < TargetSize;i++)
            TaskPos.push_back(MonsterBornPos[id[i]]);
        now = 0;
    }
    virtual void command(PCommand &cmd);
};
class FightDragonMonsterStrategy:public FightMonsterStrategy{
    protected:
        Pos targ[5];
        int Step[5];
        int targsize;
	public:
        FightDragonMonsterStrategy(string name = "FightDragonMonsterStrategy");
		virtual void NextTarget();
        virtual void command(PCommand &cmd);
};
class FightMonsterStrategy_Our_Dragon:public FightMonsterStrategy{
	public:
        FightMonsterStrategy_Our_Dragon(string name = "FightMonsterStrategy_Our_Dragon"):FightMonsterStrategy(name){
            Max = 3;
            id[0] = 30 + 2 - 2 * INFO->camp;
            id[1] = 16 + 5 - 4 * INFO->camp;
            id[2] = 16 + 4 - 4 * INFO->camp;
            id[3] = 36 + 1 - 1 * INFO->camp;
            TargetSize = 4;
            TaskPos.clear();
            for (int i = 0;i < TargetSize;i++)
                TaskPos.push_back(MonsterBornPos[id[i]]);
            now = 0;
        }
};
class FightMonsterStrategy_Enemy_Dragon:public FightMonsterStrategy{
    public:
        FightMonsterStrategy_Enemy_Dragon(string name = "FightMonsterStrategy_Enemy_Dragon"):FightMonsterStrategy(name){
            Max = 3;
            id[0] = 30 + 0 + 2 * INFO->camp;
            id[1] = 16 + 1 + 4 * INFO->camp;
            id[2] = 16 + 0 + 4 * INFO->camp;
            id[3] = 36 + 0 + 1 * INFO->camp;
            TargetSize = 4;
            TaskPos.clear();
            for (int i = 0;i < TargetSize;i++)
                TaskPos.push_back(MonsterBornPos[id[i]]);
            now = 0;
        }
        virtual void command(PCommand &cmd);
};
class FightMonsterStrategy_22_26:public FightMonsterStrategy{
	public:
        FightMonsterStrategy_22_26(string name = "FightMonsterStrategy_22_26"):FightMonsterStrategy(name){
            Max = 1;
            id[0] = 16 + 6 - 4 * INFO->camp;
            id[1] = 24 + 2 + 3 * INFO->camp;
            TargetSize = 2;
            TaskPos.clear();
            for (int i = 0;i < TargetSize;i++)
                TaskPos.push_back(MonsterBornPos[id[i]]);
            now = 0;
        }
        virtual void command(PCommand &cmd);
};
class TowerDefenceStrategy:public FightMonsterStrategy{
public:
    TowerDefenceStrategy(string name = "TowerDefenceStrategy"):FightMonsterStrategy(name){
        Max = NeedToDefence();
        if (Max == 2){
            if (OurTowerDes[1]){
                id[0] = 16 + 6 - 4 * INFO->camp;
                id[1] = 24 + 2 + 3 * INFO->camp;
                id[2] = 30 + 3 - 2 * INFO->camp;
                TargetSize = 3;
                now = 0;
            }
            else{
                id[0] = 30 + 2 - 2 * INFO->camp;
                id[1] = 16 + 7 - 4 * INFO->camp;
                TargetSize = 2;
                now = 0;
            }
        }
        else{
            if (OurTowerDes[1]){
                id[0] = 16 + 6 - 4 * INFO->camp;
                id[1] = 24 + 2 + 3 * INFO->camp;
                TargetSize = 2;
                now = 0;
            }
            else{
                id[0] = 30 + 2 - 2 * INFO->camp;
                id[1] = 16 + 7 - 4 * INFO->camp;
                TargetSize = 2;
                now = 0;
            }
        }
        TaskPos.clear();
        for (int i = 0;i < TargetSize;i++)
            TaskPos.push_back(MonsterBornPos[id[i]]);
        TaskPos.push_back(SpringPos);
    }
};
class AssembleStrategy:public Strategy{
	protected:
		Pos AssemblePos;
		int TowerNum;
        bool NeedToBack(const PUnit* hero);
        virtual int StepToReach(const PUnit* hero,Pos target);
        Strategy* SubTask[5];
        int SubTaskNum;
        void CheckChangingTower();
        bool CanStratPush();
        void FindSubTask(int id);
        void GetSubTask();
        vector<Pos> steps;
        bool CanPush;
        int FarestHero;
        int SubTaskStatus;
	public:        
        AssembleStrategy(int n,string name = "AssembleStrategy");
        virtual void command(PCommand &cmd);
};
class PushTower:public Strategy{
	protected:
		int TowerNum;
	public:
        PushTower(int n,string name = "PushTower"):Strategy(name),TowerNum(n){Max = 5;TaskPos.push_back(EnemyTower[n]);}
        virtual void command(PCommand &cmd);
};
class FightStrategy:public Strategy{
public:
    void Stimulate(Operation& op);
    FightStrategy(string name = "FightStrategy"):Strategy(name){}
    int BestPiercingArrow(Operation& op,const PUnit* myhero);
    int BestArrowsRain(Operation& op,const PUnit* myhero);
    int BestAttack(Operation& op,const PUnit* myhero);
    int BestMove(Operation& op,const PUnit* myhero,TargetEstimate* estimate);
    double Firerate(int current,const PUnit* enemy);
    virtual void command(PCommand &cmd);
};
class DetectStrategy:public Strategy{
protected:
    const Pos* DetectPos;
    bool CanBeSeenByOurHero(Pos pos);
    bool HeroHasTask[5];
    bool PosDone[MaxDetectPosSize];
    Strategy* nextTask;
public:
    DetectStrategy(Strategy* n,const Pos* p,string name = "DetectStrategy");
    virtual void command(PCommand &cmd);
};
DetectStrategy::DetectStrategy(Strategy* next,const Pos* p,string name):Strategy(name),DetectPos(p),nextTask(next){
    memset(PosDone,0,sizeof(PosDone));
    Max = 5;
    for (int i = 0; i < MaxDetectPosSize;i++)
        if (!(DetectPos[i] == Pos(-1,-1)))
            TaskPos.push_back(DetectPos[i]);
        else
            PosDone[i] = true;
}
bool DetectStrategy::CanBeSeenByOurHero(Pos pos){
    for (int i=0; i<OurHero.size(); ++i)
    {
        if (ACanSeenPos(OurHero[i],pos))
            return true;
    }
    return false;
}
void DetectStrategy::command(PCommand &cmd){
    for (int i = 0; i < MaxDetectPosSize;i++)
        if (!PosDone[i] && CanBeSeenByOurHero(DetectPos[i]))
            PosDone[i] = true;
    bool TaskFinish = true;
    for (int i = 0; i < MaxDetectPosSize;i++)
        if (!PosDone[i]) TaskFinish = false;
    if (TaskFinish){
        ChangeTask(nextTask,cmd);
        return;
    }
    memset(HeroHasTask,0,sizeof(HeroHasTask));
    int HeroRemain = num;
    int besthero,mindis2;
    while (HeroRemain){
        for (int i = 0; i < MaxDetectPosSize;i++)
            if (!PosDone[i] && HeroRemain) {
                mindis2 = Maxint;
                for(int h = 0; h < num;h++)
                    if (!HeroHasTask[h] && dis2(INFO->findUnitById(_hero[h])->pos,DetectPos[i]) < mindis2){
                        mindis2 = dis2(INFO->findUnitById(_hero[h])->pos,DetectPos[i]);
                        besthero = h;
                    }
                HeroHasTask[besthero] = true;
                Move(INFO->findUnitById(_hero[besthero]),cmd,DetectPos[i]);
                HeroRemain--;
            }
    }
}
int FightStrategy::BestPiercingArrow(Operation& op,const PUnit* myhero){
    if (!myhero->canUseSkill(Skill("PiercingArrow")))
        return 0;
    int maxvalue = 0;
    int damage = PiercingArrow_damage[myhero->findSkill("PiercingArrow")->level];
    double value;
    Pos bestPos(0,0);
    int BasePiercingArrowValue[12] = {8000,1 W,12000,3000};
    for (int i = -9;i <= 9;i++)
        for (int  j = -9;j <= 9;j++)
            if (i*i + j*j <=81 && i*i + j*j >=25){
                value = 0;//1-2
                for (int e = 0; e < EnemyHero.size();e++)
                    if (StimulateHp[EnemyHero[e]->id] > 0 && PiercingArrow_inRange(myhero->pos,myhero->pos + Pos(i,j),EnemyHero[e]->pos)){
                        value += BasePiercingArrowValue[EnemyHero[e]->typeId] *
                                (0.5 + EnemyHero[e]->max_hp / (MAX(StimulateHp[EnemyHero[e]->id],damage) + EnemyHero[e]->max_hp))
                                *(EnemyCanEscape[e]?0.7:1);
                    }
                if (value > maxvalue){
                    maxvalue = int (value);
                    bestPos = myhero->pos + Pos(i,j);
                }

            }
    if (maxvalue > 0){
        maxvalue = maxvalue - 2400 + 1200 * myhero->findSkill("PiercingArrow")->level;
        op.id = myhero->id;
        op.type = "PiercingArrow";
        op.targets.push_back(bestPos);
    }
    return maxvalue;
    //return myhero->findSkill("PiercingArrow")->level * 20 * maxvalue;

}
int FightStrategy::BestArrowsRain(Operation& op,const PUnit* myhero){
    if (!myhero->canUseSkill(Skill("ArrowsRain")))
        return 0;
    int maxvalue = 0;
    double value;
    int damage = Rainy_damage[myhero->findSkill("ArrowsRain")->level];
    Pos bestPos(0,0);
    double BasePiercingArrowValue[12] = {6000,6000,6000,0};
    for (int i = -8;i <= 8;i++)
        for (int  j = -8;j <= 8;j++)
            if (i*i + j*j <=64){
                value = 0;//1-2
                for (int e = 0; e < EnemyHero.size();e++)
                    if (StimulateHp[EnemyHero[e]->id] > 0 && dis2(myhero->pos + Pos(i,j),EnemyHero[e]->pos) <= 25 && !EnemyIntheRain[e]){
                        if (!EnemyHero[e]->findBuff("Dizzy"))
                            value += BasePiercingArrowValue[EnemyHero[e]->typeId] *
                                    (45 - dis2(myhero->pos + Pos(i,j),EnemyHero[e]->pos)) / 25;
                        else
                            value += BasePiercingArrowValue[EnemyHero[e]->typeId] * 2.5;
                    }
                for (int f = 0; f < OurHero.size();f++){
                    if (!HeroHadBeenProtected[OurHero[f]->id % 5] && dis2(myhero->pos + Pos(i,j),OurHero[f]->pos) <= 9 && OurHeroInDizzy[OurHero[f]->id % 5] > 0)
                        value += BasePiercingArrowValue[OurHero[f]->typeId] * 2.5 * OurHeroInDizzy[OurHero[f]->id % 5];
                }
                if (value > maxvalue){
                    maxvalue = int (value);
                    bestPos = myhero->pos + Pos(i,j);
                }
            }
    if (maxvalue > 0){
        maxvalue = maxvalue - 2400 + 1200 * myhero->findSkill("ArrowsRain")->level;
        op.id = myhero->id;
        op.type = "ArrowsRain";
        op.targets.push_back(bestPos);
    }
    return maxvalue;

}
double FightStrategy::Firerate(int current,const PUnit* enemy){
    double ans = 1;
    if (enemy->hp < myhero->atk - enemy->def)
        ans = Maxint;
    else for (int i = current + 1; i < num ;i++)
        if (INFO->findUnitById(_hero[i])->findSkill("Attack")->cd == 0 && dis2(enemy->pos,INFO->findUnitById(_hero[i])->pos) < INFO->findUnitById(_hero[i])->range)
            ans += 0.5;
    return ans;

}
int FightStrategy::BestAttack(Operation& op,const PUnit* myhero){
    if (!myhero->canUseSkill(Skill("Attack")))
        return 0;
    PUnits near;
    INFO->findUnitInArea(myhero->pos, myhero->range, near);
    //{"Fighter", "Archer", "Assassin", "Tower", "Spring", "Chooser", "Dog", "Bow", "Wolf", "StoneMan", "Dragon", "Roshan"};
    for (int i = 0; i < near.size(); ++i)
        if (near[i]->camp == INFO->camp ||near[i]->findBuff("Reviving") || ((*myhero)["OnlyTarget"]->val[0] >=0 && (*myhero)["OnlyTarget"]->val[0] != near[i]->id)){
            near.erase(near.begin()+i);
            --i;
        }
    int IdinGroup;
    for (int i = 0; i < num;i++)
        if (myhero->id == _hero[i])
           IdinGroup = i;
    int BaseAttackValue[12] = {1 W,1 W,1 W,3000};
    int maxvalue = 0;
    const PUnit* BestTarget;
    double value;
    const PUnit* enemy;
    for (int i = 0; i < near.size(); ++i) if (StimulateHp[near[i]->id] > 0){
        enemy = near[i];//1-2
        value = BaseAttackValue[enemy->typeId] * (1 + 0.05 * (10 - StimulateHp[near[i]->id] / (0.0001+ myhero->atk - enemy->def) / Firerate(IdinGroup,enemy)));
        if (value > maxvalue){
            maxvalue = int (value);
            BestTarget = enemy;
        }
    }
    if (maxvalue > 0){
        op.id = myhero->id;
        op.type = "Attack";
        op.targets.push_back(BestTarget->pos);
    }
    return maxvalue;
    return (myhero->atk - BestTarget->def) * maxvalue;

}
int FightStrategy::BestMove(Operation& op,const PUnit* myhero,TargetEstimate* estimate){
    op.id = myhero->id;
    op.type = "Move";
    return TargetBFS(estimate,myhero,1,op.targets,blocks_without_Tower_Rainy)->getvalue();
}
void FightStrategy::Stimulate(Operation& op){
    if (op.type == "Move")
        OurHeroPos[op.id % 5] = op.targets[op.targets.size() - 1];
    if (op.type == "Attack"){
        const PUnit* enemy = INFO->findUnitByPos(op.targets[op.targets.size() - 1]);// possible bug
        StimulateHp[enemy->id]
                -= (myhero->atk - enemy->def);
    }
    if (op.type == "PiercingArrow"){
        Pos target = op.targets[op.targets.size() - 1];
        for (int i = 0;i < EnemyHero.size();i++)
            if (PiercingArrow_inRange(myhero->pos,target,EnemyHero[i]->pos))
               StimulateHp[EnemyHero[i]->id] -= PiercingArrow_damage[myhero->findSkill("PiercingArrow")->level];
    }
    if (op.type == "ArrowsRain"){
        Pos target = op.targets[op.targets.size() - 1];
        OurArea.push_back(new PArea(0,0,INFO->camp,6,target.x,target.y,5));
        for (int i = 0;i < EnemyHero.size();i++)
            if (!EnemyIntheRain[i] && dis2(EnemyHero[i]->pos,target) <= 25){
                EnemyIntheRain[i] = true;
                StimulateHp[EnemyHero[i]->id] -= Rainy_damage[myhero->findSkill("ArrowsRain")->level];
            }
    }

}

void FightStrategy::command(PCommand &cmd){
    memset(HeroHadBeenProtected,0,sizeof(HeroHadBeenProtected));
    for (int i = 0; i < num;i++){
        myhero = INFO->findUnitById(_hero[i]);
        for (int f = 0; f < OurHero.size();f++){
            for (int j = 0; j < OurArea.size();j++)
                if (dis2(OurArea[j]->center,OurHero[f]->pos) <= 9)
                    HeroHadBeenProtected[OurHero[f]->id % 5] = true;
        }
        int mind = Maxint,minj = -1;
        for (int j = 0;j < EnemyHero.size();j++) if (dis2(myhero->pos,EnemyHero[j]->pos) < mind){
            mind = dis2(myhero->pos,EnemyHero[j]->pos);
            minj = j;
        }
        if (mind > 400)
            fightingMonster(myhero,EnemyHero[minj],cmd);
        else{
            Pos Battlepos((myhero->pos.x + EnemyHero[minj]->pos.x)/2,(myhero->pos.y + EnemyHero[minj]->pos.y)/2);
            double StrengthRate =StrengthCompare(Battlepos,144);
            int value[5] = {0};Operation op[5];
            switch (myhero->typeId){
            case 0: //fighter
                break;
            case 1:
                value[0] = BestPiercingArrow(op[0],myhero);
                value[1] = BestArrowsRain(op[1],myhero);
                value[2] = 0;
                value[3] = BestAttack(op[3],myhero);
                if (StrengthRate <= 1.5 && StrengthRate >= 0.7)
                    value[4] = BestMove(op[4],myhero,new FightingMoveEvaluation(myhero));
                if (StrengthRate > 1.5)
                    value[4] = BestMove(op[4],myhero,new RunAfterWeakEnemy(myhero));
                if (StrengthRate < 0.7){
                    for (int e = 0; e < EnemyHero.size();e++)
                        if (value[3] < 14999 && dis2(EnemyHero[e]->pos,myhero->pos) <= EnemyHero[e]->range) value[3] = 0;
                    value[4] = BestMove(op[4],myhero,new RunAwayFromPowerfulEnemy(myhero));
                }
                break;
            case 2: // Assassin
                break;
            }
            if (myhero->findBuff("Yelled")){
                value[0] = 0;
                value[1] = 0;
                value[2] = 0;
            }
            if (myhero->hp < MayHaveDamage(myhero) + 45)
                for (int i = 0; i <3;i++) value[i] *= 2;
            if (OurHeroInDizzy[myhero->id % 5] > 0){
                value[4] = int(value[3] * (1 - OurHeroInDizzy[myhero->id % 5]));
                value[3] = int(value[2] * (1 - 0.5 * OurHeroInDizzy[myhero->id % 5]));
            }
            int maxvalue = 0,maxj = 4;
            for (int j = 0; j < 5;j++)
                if (value[j] > maxvalue){
                    maxvalue = value[j];
                    maxj = j;
                }
            bool enemyescape = true;
            for (int i = 0;i <EnemyHero.size();i++)
                if (dis2(EnemyHero[i]->pos,myhero->pos) <= 81)
                    enemyescape = false;
            if (enemyescape){
                enemyescape = false;
                for (int i = 0;i <EnemyHero.size();i++)
                    if (dis2(EnemyHero[i]->pos,myhero->pos) <= 225)
                        enemyescape = true;
            }
            if (maxj == 4 && StrengthRate > 2 && enemyescape){
                RunAfterEnemyJudge[myhero->id % 5] = true;
                RunAfterEnemyRounds[myhero->id % 5]++;   //special
                if (RunAfterEnemyRounds[myhero->id % 5] >= 4){
                    erase(myhero);
                    i--;
                    continue;
                }
            }
            cmd.cmds.push_back(op[maxj]);
            Stimulate(op[maxj]);
        }
    }
}
void Strategy::ChangeTask(Strategy* T,PCommand &cmd){
    int id = Taskid();
    Task[id] = T;
    for (int i = 0;i < num;i++) Task[id]->append(INFO->findUnitById(_hero[i]));
    Task[id]->command(cmd);
}

int Strategy::StepToReach(const PUnit* hero,Pos target){
    double speed = sqrt((*hero)["Speed"]->val[1]) - 0.5;
    double range = sqrt(hero->range);
    return int((dis(hero->pos,target) - range) / speed) + 1;

}
void Strategy::Move(const PUnit* myhero,PCommand &cmd,Pos target){
    INFO->findUnitInArea(myhero->pos, myhero->view, nearBy);
    for (int i = 0; i < nearBy.size(); ++i)
        if (nearBy[i]->id > 38 || nearBy[i]->id < 16 || dis2(target,nearBy[i]->pos) <= 8)
        {
            nearBy.erase(nearBy.begin()+i);
            --i;
        }
    for (int i = 0; i < nearBy.size(); ++i){
        bool herooutit = true;
        for (int j =0; j <OurHero.size();j++)
            if (dis2(nearBy[i]->pos,OurHero[j]->pos) <= 8)
                herooutit = false;
        if (herooutit) {
            for (int x = -2;x <= 2;x++)
                for (int y = -2;y <= 2;y++)
                    if (x*x+y*y<=8){
                        blocks.push_back(nearBy[i]->pos + Pos(x,y));
                        blocks_without_Tower.push_back(nearBy[i]->pos + Pos(x,y));
                        blocks_without_Rainy.push_back(nearBy[i]->pos + Pos(x,y));
                        blocks_without_Tower_Rainy.push_back(nearBy[i]->pos + Pos(x,y));
                    }
        }
    }
    bool TowerBLock = true;
    for (int i = 0; i < 2;i++)
        if (!towerDes[i] && (dis2(myhero->pos,EnemyTower[i]) < TowerBlockRange || dis2(target,EnemyTower[i]) < TowerBlockRange))
            TowerBLock = false;
    Operation op;
    op.id = myhero->id;
    op.type = "Move";
    if (OurHeroIntheRain[myhero->id % 5]){
        if (TowerBLock)
            TargetBFS(new MoveToTarget(target,myhero),myhero,1,op.targets,blocks_without_Rainy);
        else
            TargetBFS(new MoveToTarget(target,myhero),myhero,1,op.targets,blocks_without_Tower_Rainy);

    }
    else{
        if (TowerBLock)
            SelfShortPath.findShortestPath(*MAP, myhero->pos, target, blocks, op.targets);
        else
            SelfShortPath.findShortestPath(*MAP, myhero->pos, target, blocks_without_Tower, op.targets);
    }
    if (op.targets[0] == Pos(-1,-1))
        SelfShortPath.findShortestPath(*MAP, myhero->pos, target, basic_blocks, op.targets);
    cmd.cmds.push_back(op);
}
void Strategy::Move(const PUnit* myhero,PCommand &cmd,TargetEstimate* estimate,vector<Pos>& block){
    Operation op;
    op.id = myhero->id;
    op.type = "Move";    
    TargetBFS(estimate,myhero,1,op.targets,block);
    cmd.cmds.push_back(op);
}
void Strategy::append(const PUnit* myhero){
   _hero[num++] = myhero->id;
   HeroTask[myhero->id] = this;
}
void Strategy::erase(int i){
    HeroTask[_hero[i]] = 0;
    for (int j = i;j < num - 1;j++){
        _hero[j] = _hero[j + 1];
    }
    num --;
}
void Strategy::erase(const PUnit* myhero){
    for (int i = 0;i < num;i++)
        if (_hero[i] == myhero->id){
            erase(i);
            return;
        }
}
void Strategy::fightingMonster(const PUnit* a, const PUnit* b, PCommand &cmd)
{
    int max = 0;
    for (int i = 0; i < OurHero.size();i++)
        if (dis2(OurHero[i]->pos,MonsterBornPos[b->id]) <= 324  && ang[b->id][OurHero[i]->id] > max) max = ang[b->id][OurHero[i]->id];
    if (b->id >= 16 && b->id <= 36){
        MonsterPlayer = b->player;
    }
    Operation op;
    op.id = a->id;

    vector<const PSkill*> useSkill;
    for (int i = 0; i < a->skills.size(); ++i)
    {
        const PSkill* ptr = &a->skills[i];
        if (ptr->isHeroSkill() && a->canUseSkill(ptr->typeId))
        {
            PUnits ptr_foe;
            infectedBySkill(*INFO, a->id, ptr->typeId, ptr_foe);
            if ((belongs(b->id, ptr_foe) && (ptr->maxCd <= 100 || b->isHero()))|| !strcmp(ptr->name,"Hide") || !strcmp(ptr->name,"PowerUp"))
                useSkill.push_back(ptr);
            if (ptr->typeId == Skill("ArrowsRain") && dis2(a->pos,b->pos) <= 100 && !DragonARRounds && b->typeId == 10 && (INFO->round > 35 || a->id % 5 ==4)) // dargon special
                useSkill.push_back(ptr);
        }
    }
    if (b->typeId == 6 || b->typeId == 7 || b->typeId == 9 || (b->typeId == 8 || b->typeId == 3)) useSkill.clear();
    if ((*a)["Exp"]->val[3] && a->findSkill("Attack")->cd >= 0 && b->typeId == 10 && (HeroTask[a->id] && HeroTask[a->id]->TaskName ==  "FightDragonMonsterStrategy")){
        levelUp(a, cmd);
        return;
    }
    if (useSkill.size() && (b->isHero() || (a->mp > 150)))
    {
        const PSkill* ptr = useSkill[rand()%useSkill.size()];
        op.type = ptr->name;
        if (ptr->typeId == Skill("ArrowsRain"))
            DragonARRounds = 6;
        if (ptr->needTarget())
            op.targets.push_back(b->pos);
        cmd.cmds.push_back(op);
    } else
        if (dis2(a->pos, b->pos) <= a->range)
        {
            if (a->findSkill("Attack")->cd == 1){
                if (b->range < 64 && (b->atk - a->def) * 30 > a->max_hp){                    
                    if (ang[b->id][a->id] == max && !b->isHero()){
                        Move(a,cmd,new FarAwayFromTarget(b->pos,a),PushingTower <= 0?blocks_without_Rainy:blocks_without_Tower_Rainy);
                    }
                    else if (ang[b->id][a->id] < max - 1)
                        Move(a,cmd,b->pos);
                }
                else {
                    if ((HeroTask[a->id] && HeroTask[a->id]->TaskName ==  "FightDragonMonsterStrategy"))
                        Move(a,cmd,new MoveToTarget(INFO->camp?Pos(150,150):Pos(0,0),a),blocks_without_Tower_Rainy);
                    else
                        Move(a,cmd,new DoNotWantMove(a),blocks_without_Tower_Rainy);

                }
            }
            else {
                op.type = "Attack";
                op.targets.push_back(b->pos);
                cmd.cmds.push_back(op);
            }
        } else
        {
            Move(myhero,cmd,b->pos);
        }
}
bool Strategy::FightMonster(PCommand &cmd,const PUnit* ptr){
    if ((*ptr)["Exp"]->val[3])
    {
        levelUp(ptr, cmd);
        return true;
    }
    if (ptr->findBuff("Reviving")) {
        return false;
    }
    INFO->findUnitInArea(ptr->pos, ptr->view, nearBy);
    fixPos[ptr->id] = EMPTYPOS;
    for (int i = 0; i < nearBy.size(); ++i)
    {
        if (unitEvaluation(nearBy[i]->name) * nearBy[i]->hp > nearBy[i]->max_hp * StrengthCaculate(nearBy[i]->pos,901,INFO->camp) ||
                nearBy[i]->camp == ptr->camp ||
                nearBy[i]->pos == Bow_pos[0] || nearBy[i]->pos == Bow_pos[3] ||
                nearBy[i]->findBuff("Reviving"))
        {
            nearBy.erase(nearBy.begin()+i);
            --i;
        }

    }
    if (nearBy.size())
    {
        const PUnit* ptr_foe = fixId[ptr->id]!=EMPTYID ?
                    belongs(fixId[ptr->id], nearBy) : NULL;
        if (!ptr_foe)
        {
            ptr_foe = nearBy[rand()%nearBy.size()];
            fixId[ptr->id] = ptr_foe->id;
        }
        fightingMonster(ptr, ptr_foe, cmd);
    } else
    {
        //findFoes(ptr, cmd);
        return false;
    }
    return true;
}

void PushTower::command(PCommand &cmd){
    Max = 5 - NeedToDefence();
    PushingTower = 1;
	if (towerDes[TowerNum]){
        ChangeTask(new AssembleStrategy(1 - TowerNum),cmd);
		return;
	}
	else{
		bool stillFighting = false;
        for (int i = 0;i < num;i++){
            myhero = INFO->findUnitById(_hero[i]);
            if (dis2(myhero->pos,EnemyTower[TowerNum]) <= 3000){
                stillFighting = true;
                break;
            }
        }
        if (!stillFighting){
            ChangeTask(new AssembleStrategy(TowerNum),cmd);
			return;
		}
    }
    //------
    bool CanStrongPush = INFO->findUnitByPos(EnemyTower[TowerNum]),CanPush = true;
    if (CanStrongPush){
        PUnits PushHero;
        for (int i = 0; i < num;i++)
            if (dis2(EnemyTower[TowerNum],INFO->findUnitById(_hero[i])->pos) <= 81)
                PushHero.push_back(INFO->findUnitById(_hero[i]));
        CanStrongPush = INFO->findUnitByPos(EnemyTower[TowerNum])->hp <= StrongPushingStimulate(PushHero);
        PushHero.clear();
        for (int i = 0; i < num;i++)
            if (dis2(EnemyTower[TowerNum],INFO->findUnitById(_hero[i])->pos) <= 841)
                PushHero.push_back(INFO->findUnitById(_hero[i]));
        CanPush = INFO->findUnitByPos(EnemyTower[TowerNum])->hp <= StrongPushingStimulate(PushHero,10);

    }
    bool EnemyAround[5] = {0};
    for (int i = 0;i < EnemyHero.size();i++)
        for (int j = 0;j < num;j++)
            if (dis2(INFO->findUnitById(_hero[j])->pos,EnemyHero[i]->pos) <= 100)
                EnemyAround[j] = true;
    if (!CanPush){ // dis2(EnemyHero[i]->pos,EnemyTower[TowerNum]) <= 625
        for (int j = 0;j < num;j++){
            myhero = INFO->findUnitById(_hero[j]);
            Move(myhero,cmd,new PushingDispersed(myhero),blocks_without_Tower_Rainy);

        }
        for (;num;){
            erase(0);
        }
        Task[Taskid()] = new AssembleStrategy(TowerNum);
        return;
    }
    for (int i = 0;i < num;i++)
        if (!CanStrongPush && EnemyAround[i]){
            myhero = INFO->findUnitById(_hero[i]);
            Move(myhero,cmd,new PushingDispersed(myhero),blocks_without_Tower_Rainy);
            erase(i);
            i--;
        }
    for (int i = 0;i < num;i++){
        myhero = INFO->findUnitById(_hero[i]);
        if ((dis2(myhero->pos,EnemyTower[TowerNum]) <= 81 && myhero->findSkill("Attack")->cd == 1)
                || (dis2(myhero->pos,EnemyTower[TowerNum]) <= 121 && dis2(myhero->pos,EnemyTower[TowerNum]) > 81)){
           TargetEstimate* estimate = new PushingTowerMove(myhero);
           Move(myhero,cmd,estimate,blocks_without_Tower_Rainy);
           OurHeroPos[myhero->id % 5] = estimate->getvalue();
        }
        else {
            if (!FightMonster(cmd,myhero)){
                Move(myhero,cmd,EnemyTower[TowerNum]);
            }
        }
    }
}
bool AssembleStrategy::NeedToBack(const PUnit *hero){
    return (60 - dis(hero->pos,SpringPos) / 4) * hero->max_hp > hero->hp * 100;
}

int AssembleStrategy::StepToReach(const PUnit* hero,Pos target){
    if (NeedToBack(hero))
        return Maxint;
    double speed = sqrt((*hero)["Speed"]->val[1]) - 0.5;
    double range = sqrt(hero->range);
    return int((dis(hero->pos,target) - range) / speed) + 1;

}
void AssembleStrategy::CheckChangingTower(){
    if (!towerDes[1 - TowerNum]){
        int totalHero = 0;
        int UnfitHero = 0;
        int UnfitLevel = 0;
        for (int i = 0; i < OurHero.size();i++) if (!OurHero[i]->findBuff("Reviving")){
            totalHero++;
            if (dis(OurHero[i]->pos,EnemyTower[1 - TowerNum]) + 5 < dis(OurHero[i]->pos,EnemyTower[TowerNum])){
                UnfitHero++;
                UnfitLevel += (OurHero[i]->level + 2);
            }
        }
        if (UnfitHero * 2 > totalHero)// && UnfitLevel > PushingLevel)
            TowerNum = 1 - TowerNum;
    }
}
bool AssembleStrategy::CanStratPush(){
    if (!LastRoundWantPush)
        PushingLevel = 9;
    else
        PushingLevel = 7;
    int EnemyBeginId = INFO->camp?39:44;
    for (int i = EnemyBeginId; i < EnemyBeginId + 5; i++){ // modify
        if (reviving[i] <= 0 &&
        (LastRoundsGetInfo[i] == -1
         || dis(StimulatePos[i],EnemyTower[TowerNum]) / 5 < INFO->round - LastRoundsGetInfo[i])){
            PushingLevel += (EnemyLevel[i % 5] * 2 / 3);
        }
    }
    steps.clear();
    for (int i = 0;i < num;i++){
        steps.push_back(Pos(_hero[i],StepToReach(INFO->findUnitById(_hero[i]),EnemyTower[TowerNum])));
    }
    sort(steps.begin(),steps.end());
    int TotalLevel = 0;
    FarestHero = steps.size() - 1;
    CanPush = false;
    for (int i = 0;i < steps.size();i++){
        myhero = INFO->findUnitById(steps[i].x);
        TotalLevel += (myhero->level + 0); // modify
        if (TotalLevel >= PushingLevel){
            CanPush = true;
            FarestHero = i;
             if (!(i+1 < steps.size() && steps[i + 1].y - steps[i].y <= 3))
                break;
        }
    }
    LastRoundWantPush = CanPush;
    if (steps[FarestHero].y >= 5)
        return false;
    PUnits PushingHero;
    for (int i = 0;i <= FarestHero;i++)
        PushingHero.push_back(INFO->findUnitById(steps[i].x));
    if (StrongPushingStimulate(PushingHero,10) < 1000)
        CanPush = false;
    return CanPush;
}
void AssembleStrategy::FindSubTask(int id){
    myhero = INFO->findUnitById(id);
    if (!myhero->findBuff("Reviving")){
        int MinDistance = Maxint,minj = -1;
        for (int j = 0; j < SubTaskNum;j++) if (SubTask[j] && SubTask[j]->num < SubTask[j]->Max){
            for (int k = 0; k < SubTask[j]->TaskPos.size();k++)
                if (dis2(SubTask[j]->TaskPos[k],myhero->pos) < MinDistance){
                    MinDistance = dis2(SubTask[j]->TaskPos[k],myhero->pos);
                    minj = j;
                }
        }
        if (minj != -1) SubTask[minj]->append(myhero);
    }
}
void AssembleStrategy::GetSubTask(){
    if (!OurTowerDes[0] && !OurTowerDes[1]){
        if (SubTaskStatus != 3){
            SubTask[0] = new FightMonsterStrategy(2,26,33,22,0);
            SubTask[1] = new PullMonsterStrategy_DogAndWolf;
            SubTask[2] = new FightMonsterStrategy(2,21,20,0,0);
            SubTaskNum = 3;
            SubTaskStatus = 3;
        }
        return;
    }
    if (!OurTowerDes[0] && OurTowerDes[1]){
        if (SubTaskStatus != 2){
            SubTask[0] = new FightMonsterStrategy(5,26,33,22,0);
            SubTaskNum = 1;
            SubTaskStatus = 2;
        }
        return;
    }
    if (OurTowerDes[0] && !OurTowerDes[1]){
        if (SubTaskStatus != 1){
            if (NeedToDefence()){
                SubTask[0] = new FightMonsterStrategy(3,32,21,20,0);
                SubTaskNum = 1;
            }
            else {
                SubTask[0] = new FightMonsterStrategy(2,23,35,0,0);
                SubTask[1] = new FightMonsterStrategy(3,32,21,20,0);
                SubTaskNum = 2;
            }
            SubTaskStatus = 1;
        }
        return;
    }
}

void AssembleStrategy::command(PCommand &cmd){
    Max = 5 - NeedToDefence();
    PushingTower = 0;
    if (towerDes[TowerNum] && !towerDes[1 - TowerNum]){
        ChangeTask(new AssembleStrategy(1 - TowerNum),cmd);
		return;
    }
    CheckChangingTower();
    if (CanStratPush()){
        if (DetectBeforePushing())
            ChangeTask(new DetectStrategy(new PushTower(TowerNum),PushingDetectPos[INFO->camp][TowerNum]),cmd);
        else
            ChangeTask(new PushTower(TowerNum),cmd);
        return;
    }
    if (NeedToDefence()){// assassin{
        while (5 - NeedToDefence() < num){
            myhero = INFO->findUnitById(steps[num - 1].x);
            erase(myhero);
            steps.pop_back();
            int DefenceTaskid = -1;
            for (int i = 0;i < TotalTask;i++)
                if (Task[i] && Task[i]->TaskName == "TowerDefenceStrategy")
                    DefenceTaskid = i;
            if (DefenceTaskid  == -1){
                Task[TotalTask++] = new TowerDefenceStrategy;
                DefenceTaskid  = TotalTask - 1;
            }
            Task[DefenceTaskid]->append(myhero);
        }
        if (FarestHero >= num)
            FarestHero = num - 1;
    }
    GetSubTask();
    for (int i = 0;i < num;i++){
        myhero = INFO->findUnitById(steps[i].x);
        if (NeedToBack(myhero))
            Move(myhero,cmd,SpringPos);
        else if (!CanPush || steps[i].y < steps[FarestHero].y - 1) {
            FindSubTask(steps[i].x);
        }
        else
            Move(myhero,cmd,EnemyTower[TowerNum]);
    }
    for (int i = 0;i < SubTaskNum;i++)
        if (SubTask[i] && SubTask[i]->num > 0){
            SubTask[i]->command(cmd);
            SubTask[i]->clear();
        }
    for (int i = 0; i < num;i++)
        HeroTask[_hero[i]] = this;

}
AssembleStrategy::AssembleStrategy(int n,string name):Strategy(name){
    Max = 5;
    TowerNum = n;
    TaskPos.clear();
    TaskPos.push_back(EnemyTower[n]);
    SubTaskStatus = -1;
}
FightDragonMonsterStrategy::FightDragonMonsterStrategy(string name):FightMonsterStrategy(name){
    memset(Step,0,sizeof(Step));
    targsize = 4;
    targ[0] = Pos(13,89);
    targ[1] = Pos(26,38);
    targ[2] = Pos(19,23);
    targ[3] = Pos(5,5);
    if (INFO->camp){
        for (int i = 0; i < targsize;i++)
            targ[i] = Pos(150,150) - targ[i];
    }
    Max = 3;
    id[0] = 36 + 0 + 1 * INFO->camp;
    //id[1] = 16 + 5 - 4 * INFO->camp;
    //id[2] = 36 + 1 - 1 * INFO->camp;
    TargetSize = 1;
    TaskPos.clear();
    for (int i = 0;i < TargetSize;i++)
        TaskPos.push_back(MonsterBornPos[id[i]]);
	now = 0;
}
void FightDragonMonsterStrategy::NextTarget(){
	now++;
}
void FightMonsterStrategy::NextTarget(){
    now++;
    if (now >= TargetSize) now = now % TargetSize;
}
void FightDragonMonsterStrategy::command(PCommand &cmd){
    if (TargetCanNotAttack()){
        NextTarget();
    }
    if (now >= TargetSize){
        int id = Taskid();
        Task[id] = new FightMonsterStrategy_Enemy_Dragon();
        for (int i = 0; i < 3 && i < num;i++){
            Task[id]->append(INFO->findUnitById(_hero[i]));
        }
        Task[id]->command(cmd);
        return;
    }
    if (now){
        FightMonsterStrategy::command(cmd);
        return;
    }
    for (int i = 0; i < num;i++){
        myhero = INFO->findUnitById(_hero[i]);
        if (!INFO->findUnitById(id[now])){
            if ((*myhero)["Exp"]->val[3] && myhero->id % 5 != 3){
                levelUp(myhero, cmd);
            }
            else{
                while (Step[myhero->id % 5] < targsize && ACanSeenPos(myhero,targ[Step[myhero->id % 5]]))
                    Step[myhero->id % 5]++;
                Move(myhero,cmd,new MoveToTarget(targ[Step[myhero->id % 5]],myhero),blocks_without_Tower_Rainy);
                //Move(myhero,cmd,MonsterBornPos[id[now]]);
            }
        }
        else{
            fightingMonster(myhero,INFO->findUnitById(id[now]),cmd);
        }
    }
}
string MonsterNameById(int id){
    if (id < 16) return "";
    if (id < 24) return "Dog";
    if (id < 30) return "Bow";
    if (id < 34) return "Wolf";
    if (id < 36) return "StoneMan";
    if (id < 38) return "Dragon";
    if (id == 38) return "Roshan";
    return "";
}

bool FightMonsterStrategy::TargetCanNotAttack(){
    const PUnit* t= INFO->findUnitById(id[now]);
    int minstep = Maxint;
    for (int i = 0; i < num;i++){
        myhero = INFO->findUnitById(_hero[i]);
        if (StepToReach(myhero,MonsterBornPos[id[now]]) < minstep)
            minstep = StepToReach(myhero,MonsterBornPos[id[now]]);
    }
    if (reviving[id[now]] > 3 + minstep)
        return true;
    for (int i = 0; i < 2;i++)
        if (!OurTowerDes[i] && dis2(MonsterBornPos[id[now]],FriendlyTower[i]) < 625)
            return false;
    if (t && t->max_hp * StrengthCaculate(t->pos,901,INFO->camp) < t->hp * unitEvaluation(t->name))
        return true;
    if (!t){
        for (int i = 0; i < num;i++){
            myhero = INFO->findUnitById(_hero[i]);
            if (ACanSeenPos(myhero,MonsterBornPos[id[now]]) && dis2(myhero->pos,MonsterBornPos[id[now]]) <= 25)
                return true;
        }
        double mindis = Maxint;
        for (int i = 0;i < num;i++)
            if (dis(INFO->findUnitById(_hero[i])->pos,MonsterBornPos[id[now]]) < mindis)
                mindis = dis(INFO->findUnitById(_hero[i])->pos,MonsterBornPos[id[now]]);
        int totalLevel = 0;
        for (int i = 0;i < num;i++)
            if (dis(INFO->findUnitById(_hero[i])->pos,MonsterBornPos[id[now]]) < mindis + 13)
                totalLevel += (3 + INFO->findUnitById(_hero[i])->level);
        if (totalLevel < unitEvaluation(MonsterNameById(id[now])))
            return true;
    }
    return false;
}
void FightMonsterStrategy::command(PCommand &cmd){
    if (TargetCanNotAttack()){
		NextTarget();
	}
    if (now >= TargetSize){
		return;
	}
    const PUnit* Monster;
    for (int i = 0; i < num;i++){
        myhero = INFO->findUnitById(_hero[i]);

        if (!FightMonster(cmd,myhero)){
            Monster = INFO->findUnitById(id[now]);
            if (!Monster)
                Move(myhero,cmd,MonsterBornPos[id[now]]);
            else
                fightingMonster(myhero,Monster,cmd);
        }
    }
}
void FightMonsterStrategy_Enemy_Dragon::command(PCommand &cmd){
    for (int i = 0; i < num;i++)
        if (dis2(INFO->findUnitById(_hero[i])->pos,SpringPos) > 2500){
            FightMonsterStrategy::command(cmd);
            return;
        }
    ChangeTask(new FightMonsterStrategy_Our_Dragon,cmd);

}
void PullMonsterStrategy_26_22::command(PCommand &cmd){
    if (INFO->findUnitById(_hero[0]) && INFO->findUnitById(_hero[0])->level > 3)
        ChangeTask(new FightMonsterStrategy_22_26,cmd);
    else
        PullMonsterStrategy::command(cmd);
}
void FightMonsterStrategy_22_26::command(PCommand &cmd){
    if (INFO->findUnitById(_hero[0]) && INFO->findUnitById(_hero[0])->level < 4)
        ChangeTask(new PullMonsterStrategy_26_22,cmd);
    else
        FightMonsterStrategy::command(cmd);
}

void PullMonsterStrategy_DogAndWolf::command(PCommand &cmd){
    for (int i = 0;i < 5;i++)
        if (reviving[id[0]] > 10 && reviving[id[1]] > 10 && HeroTask[OurHero[i]->id] && HeroTask[OurHero[i]->id]->TaskName == "FightMonsterStrategy_Our_Dragon"){
            TargetSize = 3;
        }
    PullMonsterStrategy::command(cmd);
    if (now < 2)
        TargetSize = 2;
}
void PullMonsterStrategy::command(PCommand &cmd){
    if (PullBowSign) PullBowSign--;    
    if (TargetCanNotAttack()){
        NextTarget();
    }
    if (now >= TargetSize){
        return;
    }
    for (int i = 0; i < num;i++){        
        myhero = INFO->findUnitById(_hero[i]);
        if (!INFO->findUnitById(id[now])){
            if ((*myhero)["Exp"]->val[3])
                levelUp(myhero, cmd);
            else if (!PullBowSign)
                Move(myhero,cmd,MonsterBornPos[id[now]]);
            else{
                Pos mypos = INFO->camp?Pos(84,35):Pos(66,115);
                Pos targetPos = INFO->camp?Pos(82,35):Pos(68,115);
                if (dis2(mypos,myhero->pos) < 3)
                    Move(myhero,cmd,targetPos);
                else
                    Move(myhero,cmd,new DoNotWantMove(myhero),blocks_without_Tower_Rainy);
            }
        }
        else {
            const PUnit* Monster = INFO->findUnitById(id[now]);
            bool hatest = true;
            for (int f = 0;f < OurHero.size();f++)
                if (dis2(OurHero[f]->pos,MonsterBornPos[id[now]]) <= 324  && OurHero[f]->id != _hero[i] && ang[id[now]][OurHero[f]->id] > ang[id[now]][_hero[i]]) {
                    hatest = false;
                    break;
                }
            if (id[now] == 26 || id[now] == 29){
                Pos StartPos = INFO->camp?Pos(88,35):Pos(62,115);
                if (ang[id[now]][_hero[i]] == 0){
                    PullBowSign = 4;
                    if (myhero->pos != StartPos && hatest){
                        Move(myhero,cmd,StartPos);                        
                    }
                    else
                        fightingMonster(myhero,Monster,cmd);
                }
                else {
                    if (dis2(Monster->pos,MonsterBornPos[id[now]]) >= 81 &&
                            (Monster->hp <= Tower_atk + myhero->atk - Monster->def * 2 || Monster->hp > Tower_atk * 2 + myhero->atk - Monster->def * 3))
                        fightingMonster(myhero,Monster,cmd);
                    else
                        Move(myhero,cmd,new PullBowMove(TowerNum,id[now],myhero),blocks_without_Rainy);
                }

            }
            else if (dis2(MonsterBornPos[id[now]],FriendlyTower[TowerNum]) > 625 || ang[id[now]][_hero[i]] == 0 || !hatest
                    || Monster->hp <= Tower_atk + myhero->atk - Monster->def * 2)
                fightingMonster(myhero,Monster,cmd);
            else{
                Pos MonsterPos;
                if (dis2(myhero->pos,Monster->pos) <= Monster->range)
                    MonsterPos = Monster->pos;
                else{
                    vector<Pos> ppp;
                    TargetEstimate* estimate = TargetBFS(new MonsterPosStimulate(myhero->pos,Monster),Monster,1,ppp,blocks_without_Tower_Rainy);
                    MonsterPos = estimate->gettarget();
                }
                Move(myhero,cmd,new PullMonsterMove(TowerNum,id[now],myhero,MonsterPos),blocks_without_Rainy);
            }
            if (dis2(FriendlyTower[TowerNum],Monster->pos) <= 100 && Monster->hp <= Tower_atk + myhero->atk - Monster->def * 2){
                reviving[id[now]] = monsters[Monster->typeId - 6].reviving_time;
            }
        }
    }
}
//---------------------------------------------------------------------------------------------------------------advicer.cc
class Advicer{
public:
    virtual bool advice() = 0;
    int Advicersid(){for (int i = 0; i < TotalAdvicer;i++) if (this == Advicers[i]) return i; return -1;}
};
class FirstPushAdvicer: public Advicer{
public:
    virtual bool advice();    
};
class FightAdvicer: public Advicer{
protected:
    bool NeedToFight(Pos,Pos);
public:
    virtual bool advice();
};
class FindTaskAdvicer: public Advicer{
public:
    virtual bool advice();
};
bool FindTaskAdvicer::advice(){
    for (int i = 0; i < OurHero.size();i++)
        if (OurHero[i]->findBuff("Reviving") && HeroTask[OurHero[i]->id])
                HeroTask[OurHero[i]->id]->erase(OurHero[i]);
    for (int i = 0; i < OurHero.size();i++)
        if (!OurHero[i]->findBuff("Reviving") && !HeroTask[OurHero[i]->id]){
            int MinDistance = Maxint,minj = -1;
            for (int j = 1; j < TotalTask;j++) if (Task[j] && Task[j]->num < Task[j]->Max){
                for (int k = 0; k < Task[j]->TaskPos.size();k++)
                    if (dis2(Task[j]->TaskPos[k],OurHero[i]->pos) < MinDistance){
                        MinDistance = dis2(Task[j]->TaskPos[k],OurHero[i]->pos);
                        minj = j;
                    }
            }
            if (minj != -1) Task[minj]->append(OurHero[i]);
        }
}
bool FightAdvicer::NeedToFight(Pos MyPos,Pos EnemyPos){
    return dis2(MyPos,EnemyPos) <= ReinforceDistance
            || (dis2(MyPos,EnemyPos) <= DefendTowerDistance &&
                (dis2(FriendlyTower[0],EnemyPos) < 144
                 || dis2(FriendlyTower[1],EnemyPos) < 144));
}
bool FightAdvicer::advice(){    
    if (EnemyHero.size() == 0){
        if (Task[0]->num > 0)
            for (int i = 0; i < Task[0]->num;i++){
                Task[0]->erase(i);
                i--;
            }
        return false;
    }
    bool needfight;
    for (int i = 0; i < OurHero.size();i++) {
        needfight = false;
        for (int j = 0; j < EnemyHero.size();j++)
            if (NeedToFight(OurHero[i]->pos,EnemyHero[j]->pos)){
                needfight = true;
                break;
            }
        if (OurHeroInDizzy[OurHero[i]->id % 5] < EPS && !OurHero[i]->canUseSkill(Skill("ArrowsRain")) && !OurHero[i]->canUseSkill(Skill("PiercingArrow"))
            && OurHero[i]->hp - MayHaveDamage(OurHero[i]) > 0 && OurHero[i]->hp - MayHaveDamage(OurHero[i]) <= OurHero[i]->max_hp/5
            && (60 - dis(OurHero[i]->pos,SpringPos) / 4) * OurHero[i]->max_hp > OurHero[i]->hp * 100)
            needfight = false;
        if (HeroTask[OurHero[i]->id] && HeroTask[OurHero[i]->id]->TaskName == "FightDragonMonsterStrategy")
            needfight = false;
        if (needfight != (HeroTask[OurHero[i]->id] && HeroTask[OurHero[i]->id]->TaskName == "FightStrategy") ){
            if (needfight){
                if (!HeroTask[OurHero[i]->id] || HeroTask[OurHero[i]->id]->TaskName != "PushTower" || dis2(HeroTask[OurHero[i]->id]->TaskPos[0],OurHero[i]->pos) > 100){
                    if (HeroTask[OurHero[i]->id])HeroTask[OurHero[i]->id]->erase(OurHero[i]);
                    Task[0]->append(OurHero[i]);
                }
            }
            else
                Task[0]->erase(OurHero[i]);
        }

    }
    return true;
}
bool FirstPushAdvicer::advice(){
    int totalLevel = 0;
    for (int i = 0;i < OurHero.size();i++) if (HeroTask[OurHero[i]->id] && HeroTask[OurHero[i]->id]->TaskName.find("Monster") != string::npos){
        totalLevel += (OurHero[i]->level + 2);
    }
    if (totalLevel >= PushingLevel){
        Task[TotalTask++] = new AssembleStrategy(1);
        for (int i = 0;i < OurHero.size();i++) if (HeroTask[OurHero[i]->id] && HeroTask[OurHero[i]->id]->TaskName.find("Monster") != string::npos){
            HeroTask[OurHero[i]->id]->erase(OurHero[i]);
            Task[TotalTask - 1]->append(OurHero[i]);
        }
        Advicers[Advicersid()] = 0;
        for (int i = 1; i < TotalTask - 1;i++) if (Task[i]->TaskName.find("Monster") != string::npos){
            Task[i] = 0;
        }
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------main.cc
void init()
{
    for (int i = 0; i < 50;i++)
        LastRoundsGetInfo[i] = -1;
    blocks.clear();
    const Pos* tower = INFO->camp ? Player1_tower_pos : Player0_tower_pos;
    for (int i = 0; i < 2; ++i)
        blocks.push_back(tower[i]);
    memset(fixId, EMPTYID, sizeof fixId);
    towerDes[0] = towerDes[1] = false;
    SpringPos = INFO->camp ? Pos(145,5):Pos(5,145);
    for (int i = 0; i < 5;i++) EnemyHeroType[i] = -1;
    for (int i = 0; i < MAXID; ++i) fixPos[i] = EMPTYPOS;
    for (int i = 0;i < 8;i++) MonsterBornPos[i + 16] = Dog_pos[i];
    for (int i = 0;i < 6;i++) MonsterBornPos[i + 24] = Bow_pos[i];
    for (int i = 0;i < 4;i++) MonsterBornPos[i + 30] = Wolf_pos[i];
    for (int i = 0;i < 2;i++) MonsterBornPos[i + 34] = StoneMan_pos[i];
    for (int i = 0;i < 2;i++) MonsterBornPos[i + 36] = Dragon_pos[i];
    for (int i = 0;i < 1;i++) MonsterBornPos[i + 38] = Roshan_pos[i];
    TotalAdvicer = 3;
    Advicers[0] = new FightAdvicer;
    Advicers[1] = new FindTaskAdvicer;
    Advicers[2] = new FirstPushAdvicer;
}
void prepared(){
    if (DragonARRounds) DragonARRounds--;
    OurHero.clear();
    EnemyHero.clear();
    for (int i = 16; i < 50;i++) if (reviving[i]){
        reviving[i]--;
        int EnemyBeginId = INFO->camp?39:44;
        if (reviving[i] == 0 && (i >= EnemyBeginId && i < EnemyBeginId + 5)){
            StimulatePos[i] = Pos(150,150)- SpringPos;
            LastRoundsGetInfo[i] = INFO->round;
        }
    }
    for (int i = 0;i < 5;i++){
        EnemyExp[i] += 2;
        while (EnemyExp[i] >= UpLevel[EnemyLevel[i]] && EnemyLevel[i] < 9) EnemyLevel[i]++;
    }
    for (int i = 0; i < INFO->units.size();i++){
        if (INFO->units[i].id >= 16 && INFO->units[i].id <= 48){
            StimulateHp[INFO->units[i].id] = INFO->units[i].hp;
            StimulatePos[INFO->units[i].id] = INFO->units[i].pos;
            if (INFO->units[i].findBuff("Reviving"))
                reviving[INFO->units[i].id] = INFO->units[i].findBuff("Reviving")->timeLeft;
            else
                reviving[INFO->units[i].id] = 0;
            LastRoundsGetInfo[INFO->units[i].id] = INFO->round;
        }
    }
    for (int i = 0; i < 2;i++){
        if (INFO->findUnitByPos(FriendlyTower[i]) && INFO->findUnitByPos(FriendlyTower[i])->typeId == 3)
            OurTowerDes[i] = false;
        else
            OurTowerDes[i] = true;
    }
    INFO->findUnitByPlayer(INFO->player, ours);
    INFO->findAreaByCamp(INFO->camp,OurArea);
    INFO->findAreaByCamp(1 - INFO->camp,EnemyArea);
    for (int i=0; i<ours.size(); ++i)
    {
        const PUnit* ptr = ours[i];
        if (ptr->isHero()){
            OurHero.push_back(ptr);
            OurHeroPos[ptr->id % 5] = ptr->pos;
            for (int i = 0; i < 2; ++i)
                if (dis2(ptr->pos, EnemyTower[i]) < 100 && !INFO->findUnitByPos(EnemyTower[i]) &&
                        MAP->height[ptr->pos.x][ptr->pos.y] + 2 > MAP->height[EnemyTower[i].x][EnemyTower[i].y])
                    towerDes[i] = true;
        }
    }
    for (int i = 0; i < INFO->units.size();++i){
        if (INFO->units[i].isHero() && INFO->player != INFO->units[i].player){
            EnemyHero.push_back(&INFO->units[i]);
        }
    }
    for (int i = 0;i < EnemyHero.size();i++)
        EnemyCanEscape[i] = (EnemyHero[i]->canUseSkill(Skill("Charge")) || EnemyHero[i]->canUseSkill(Skill("Blink")));

    if (INFO->round == 1){
        TotalTask = 4;
        Task[0] = new FightStrategy;
        Task[1] = new FightDragonMonsterStrategy;
        Task[2] = new PullMonsterStrategy_DogAndWolf;
        Task[3] = new PullMonsterStrategy_26_22;
        Task[1]->append(OurHero[0]);
        Task[2]->append(OurHero[1]);
        Task[3]->append(OurHero[2]);
        for (int i=3; i<OurHero.size(); ++i) Task[1]->append(OurHero[i]);
    }
    calAng();
    for (int i = 0; i < 3;i++) EnemyHeroTypeNum[i] = 0;
    for (int i = 0;i < EnemyHero.size();i++){ //------------------------------------------
        EnemyHeroType[EnemyHero[i]->id % 5] =EnemyHero[i]->typeId;
    }
    for (int i = 0;i < 5;i++)if (EnemyHeroType[i] != -1) EnemyHeroTypeNum[EnemyHeroType[i]]++;
    for (int i = 0;i < EnemyHero.size();i++){
        const PUnit* Enemy = EnemyHero[i];
        if (Enemy->findBuff("Rainy") && Enemy->findBuff("Rainy")->timeLeft > 0)
            StimulateHp[Enemy->id] -= Rainy_damage[Enemy->findBuff("Rainy")->level];
        EnemyIntheRain[i] = false;
        for (int j = 0;j < OurArea.size();j++)
            if (dis(Enemy->pos,OurArea[j]->center) <= OurArea[j]->radius)
                EnemyIntheRain[i] = true;
    }
    for (int i = 0;i < OurHero.size();i++){
        OurHeroIntheRain[OurHero[i]->id % 5] = false;
        OurHeroInDizzy[OurHero[i]->id % 5] = OurHero[i]->findBuff("Dizzy")?1:0;
        for (int j = 0;j < EnemyArea.size();j++)
            if (dis(OurHero[i]->pos,EnemyArea[j]->center) <= EnemyArea[j]->radius)
                OurHeroIntheRain[OurHero[i]->id % 5] = true;
    }
    for (int i = 0;i < EnemyHero.size();i++)
        if (EnemyHero[i]->canUseSkill(Skill("Charge"))){
            int targetnum = 0;
            for (int j = 0;j < OurHero.size();j++)
                if (dis2(EnemyHero[i]->pos,OurHero[j]->pos) <= 81)
                    targetnum++;
            for (int j = 0;j < OurHero.size();j++)
                if (dis2(EnemyHero[i]->pos,OurHero[j]->pos) <= 81)
                    if (OurHeroInDizzy[OurHero[j]->id % 5] < 1){
                        OurHeroInDizzy[OurHero[j]->id % 5] += 1.00/targetnum;
                        if (OurHeroInDizzy[OurHero[j]->id % 5] > 1)
                            OurHeroInDizzy[OurHero[j]->id % 5] = 1;

                    }
        }
    for (int i = 0;i < EnemyHero.size();i++){
        EnemyExp[EnemyHero[i]->id % 5] = (*EnemyHero[i])["Exp"]->val[0];
        EnemyLevel[EnemyHero[i]->id % 5] = (*EnemyHero[i])["Exp"]->val[2];
    }
    if (PushingTower >= 0)
       TowerBlockRange = 100;
    else
       TowerBlockRange = 144;
    GetBlocks();
    for (int i = 0;i < 5;i++)
        RunAfterEnemyJudge[i] = false;
    TowerAttackValue = 12500;
    for (int i = 0; i < OurHero.size();i++)
        for (int j = 0;j < 1;j++)
        if (OurHeroInDizzy[OurHero[i]->id % 5] > EPS && !towerDes[j] && dis2(OurHero[i]->pos,EnemyTower[j]) <= 100)
            TowerAttackValue  = 0;
    if (TowerAttackValue){
        TowerAttackValue = 0;
        for (int i = 0; i < EnemyHero.size();i++)
            for (int j = 0;j < 1;j++)
                if (!towerDes[j] && dis2(EnemyHero[i]->pos,EnemyTower[j]) > 8 && dis2(EnemyHero[i]->pos,EnemyTower[j]) < 900)
                    TowerAttackValue = 900;

    }
}
void player_ai(const PMap &map, const PPlayerInfo &info, PCommand &cmd)
{
    srand(time(0));
    cmd.cmds.clear();
    MAP = &map;
    INFO = &info;
    if (info.round == 0){
        chooseHero(info, cmd);
        init();
        EnemyTower = INFO->camp ? Player0_tower_pos : Player1_tower_pos;
        FriendlyTower = INFO->camp ? Player1_tower_pos : Player0_tower_pos;
    }
    else{
        prepared();
        for (int i = 0; i < TotalAdvicer;i++) if (Advicers[i]){
            if (Advicers[i]->advice()){
                switch (i){
                    case 0 :
                        cout << Task[0]->TaskName << endl;
                        Task[0]->command(cmd);                        
                        break;
                }
            }
        }
        for (int i = 1; i < TotalTask;i++) if (Task[i]){
            if (Task[i]->num > 0)
                Task[i]->command(cmd);
        }
        int EnemyBeginid = INFO->camp?39:44;
        for (int i = EnemyBeginid;i < EnemyBeginid + 5;i++)
            if (StimulateHp[i] <= 0){
                reviving[i] = (EnemyLevel[i % 5] + 1) * 5;
                if (INFO->findUnitById(i))
                    StimulateHp[i] = INFO->findUnitById(i)->max_hp;
                else
                    StimulateHp[i] = 250 + 25 * EnemyLevel[i % 5];
            }
        for (int i = 0;i < 5;i++)
            if (!RunAfterEnemyJudge[i])
                RunAfterEnemyRounds[i] = 0;
    }
}
