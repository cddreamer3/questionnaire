#pragma warning (disable:4786)
#pragma warning (disable:4503)

#include "RailSystem.h"

void RailSystem::reset(void) // 重置cities
{
    map<string, City *>::iterator it;
    for (it = cities.begin(); it != cities.end(); it++) {
        it->second->visited = false;
        it->second->total_fee = 0;
        it->second->total_distance = 0;
        it->second->from_city = "";
    }
}

RailSystem::RailSystem(string const &filename) //构造方法，主要是载入信息
{
    load_services(filename);
}

void RailSystem::load_services(string const &filename) {
    ifstream inf(filename.c_str());
    string from, to;
    int fee, distance;
    while (inf.good()) {
        inf >> from >> to >> fee >> distance;//文件的格式为： “起始点 目的地 费用 路程”
        if (inf.good()) {
            Service *neib = new Service(to, fee, distance);
            map<string, list<Service *> >::iterator it;
            // 看起始点是否已经载，如过载入，只要添加目的地就可，否则要新建
            for (it = outgoing_services.begin(); it != outgoing_services.end(); it++) {
                if (from == it->first) {
                    break;
                }
            }
            // 起始点没有载入 在outgoing_services中新建一项
            if (it == outgoing_services.end()) {
                list<Service *> services;//此两条语句不能颠倒次序
                services.push_back(neib);//否则出错
                outgoing_services[from] = services;
                City *city = new City(from);
                cities[from] = city;
            } else//起始点已经载入
            {
                it->second.push_back(neib);
            }
        }
    }
    inf.close();
}

RailSystem::~RailSystem(void) { //析构函数，删除程序运行中申请的空间
    map<string, City *>::iterator it1;
    for (it1 = cities.begin(); it1 != cities.end(); it1++) {
        delete it1->second;
    }
    map<string, list<Service *> >::iterator it2;
    for (it2 = outgoing_services.begin(); it2 != outgoing_services.end(); it2++) {
        while (!it2->second.empty()) {
            delete it2->second.front();
            it2->second.pop_front();
        }
    }

}

//打印最佳路径的信息
void RailSystem::output_cheapest_route(const string &from, const string &to, ostream &out) {
    reset();
    pair<int, int> totals = calc_route(from, to);
    if (totals.first == INT_MAX) {
        out << "There is no route from " << from << " to " << to << "\n";
    } else {
        out << "The cheapest route from " << from << " to " << to << "\n";
        out << "costs " << totals.first << " euros and spans " << totals.second
            << " kilometers\n";
        cout << recover_route(to) << "\n\n";
    }
}

bool RailSystem::is_valid_city(const string &name) {
    return cities.count(name) == 1;
}

//计算，Dijkstra算法实现
pair<int, int> RailSystem::calc_route(string from, string to) {
    cities[from]->visited = true; //起始点
    //采用广度优先的遍历方法，用queue来储存将要访问的点
    queue<City *> q;
    q.push(cities[from]);
    while (!q.empty()) {
        string f_city = q.front()->name;
        q.pop();
        list<Service *> l = outgoing_services[f_city];// 该点周边的交通情况
        while (!l.empty()) {
            Service *d_city = l.front(); //找到第一个与该点连接的城市
            //如果该城市没被访问，则访问
            if (cities[d_city->destination]->visited == false) {
                cities[d_city->destination]->visited = true;
                cities[d_city->destination]->total_fee = cities[f_city]->total_fee + d_city->fee;
                cities[d_city->destination]->total_distance = cities[f_city]->total_distance + d_city->distance;
                cities[d_city->destination]->from_city = f_city;
                q.push(cities[d_city->destination]);
            }
                //如果被访问了，那么查看一下现在的路径是否比原来的好，如果好，则替换
            else { //同样采用广度优先的遍历方法
                queue<City *> change_q;
                change_q.push(cities[f_city]);
                while (!change_q.empty()) {
                    string change_city = change_q.front()->name;
                    change_q.pop();
                    list<Service *> change_l = outgoing_services[f_city];
                    while (!change_l.empty()) {
                        Service *ch_ser = change_l.front();
                        //如果现在的路径比以前的好，且是在被访问了的城市之间，那么替换。
                        if ((cities[f_city]->total_fee + ch_ser->fee < cities[ch_ser->destination]->total_fee) &&
                            (cities[ch_ser->destination]->visited == true)) {//更新城市信息
                            cities[ch_ser->destination]->total_fee = cities[f_city]->total_fee + ch_ser->fee;
                            cities[ch_ser->destination]->from_city = f_city;
                            cities[ch_ser->destination]->total_distance =
                                    cities[f_city]->total_distance + ch_ser->distance;
                            change_q.push(cities[ch_ser->destination]);
                        }
                        change_l.pop_front();
                    }
                }
            }
            l.pop_front();
        }
    }
//返回最佳路径的费用和长度。
    if (cities[to]->visited) {
        return pair<int, int>(cities[to]->total_fee, cities[to]->total_distance);
    } else {
        return pair<int, int>(INT_MAX, INT_MAX);
    }
}

//最佳路径的走法
string RailSystem::recover_route(const string &city) {
    string route = cities[city]->from_city;
    string print = city;
    while (route != "") {
        print = route + " to " + print;
        route = cities[route]->from_city;
    }
    return print;
}
