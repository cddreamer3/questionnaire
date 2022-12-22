#pragma warning (disable:4786)
#pragma warning (disable:4503)

#include "RailSystem.h"

void RailSystem::reset(void) // ����cities
{
    map<string, City *>::iterator it;
    for (it = cities.begin(); it != cities.end(); it++) {
        it->second->visited = false;
        it->second->total_fee = 0;
        it->second->total_distance = 0;
        it->second->from_city = "";
    }
}

RailSystem::RailSystem(string const &filename) //���췽������Ҫ��������Ϣ
{
    load_services(filename);
}

void RailSystem::load_services(string const &filename) {
    ifstream inf(filename.c_str());
    string from, to;
    int fee, distance;
    while (inf.good()) {
        inf >> from >> to >> fee >> distance;//�ļ��ĸ�ʽΪ�� ����ʼ�� Ŀ�ĵ� ���� ·�̡�
        if (inf.good()) {
            Service *neib = new Service(to, fee, distance);
            map<string, list<Service *> >::iterator it;
            // ����ʼ���Ƿ��Ѿ��أ�������룬ֻҪ���Ŀ�ĵؾͿɣ�����Ҫ�½�
            for (it = outgoing_services.begin(); it != outgoing_services.end(); it++) {
                if (from == it->first) {
                    break;
                }
            }
            // ��ʼ��û������ ��outgoing_services���½�һ��
            if (it == outgoing_services.end()) {
                list<Service *> services;//��������䲻�ܵߵ�����
                services.push_back(neib);//�������
                outgoing_services[from] = services;
                City *city = new City(from);
                cities[from] = city;
            } else//��ʼ���Ѿ�����
            {
                it->second.push_back(neib);
            }
        }
    }
    inf.close();
}

RailSystem::~RailSystem(void) { //����������ɾ����������������Ŀռ�
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

//��ӡ���·������Ϣ
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

//���㣬Dijkstra�㷨ʵ��
pair<int, int> RailSystem::calc_route(string from, string to) {
    cities[from]->visited = true; //��ʼ��
    //���ù�����ȵı�����������queue�����潫Ҫ���ʵĵ�
    queue<City *> q;
    q.push(cities[from]);
    while (!q.empty()) {
        string f_city = q.front()->name;
        q.pop();
        list<Service *> l = outgoing_services[f_city];// �õ��ܱߵĽ�ͨ���
        while (!l.empty()) {
            Service *d_city = l.front(); //�ҵ���һ����õ����ӵĳ���
            //����ó���û�����ʣ������
            if (cities[d_city->destination]->visited == false) {
                cities[d_city->destination]->visited = true;
                cities[d_city->destination]->total_fee = cities[f_city]->total_fee + d_city->fee;
                cities[d_city->destination]->total_distance = cities[f_city]->total_distance + d_city->distance;
                cities[d_city->destination]->from_city = f_city;
                q.push(cities[d_city->destination]);
            }
                //����������ˣ���ô�鿴һ�����ڵ�·���Ƿ��ԭ���ĺã�����ã����滻
            else { //ͬ�����ù�����ȵı�������
                queue<City *> change_q;
                change_q.push(cities[f_city]);
                while (!change_q.empty()) {
                    string change_city = change_q.front()->name;
                    change_q.pop();
                    list<Service *> change_l = outgoing_services[f_city];
                    while (!change_l.empty()) {
                        Service *ch_ser = change_l.front();
                        //������ڵ�·������ǰ�ĺã������ڱ������˵ĳ���֮�䣬��ô�滻��
                        if ((cities[f_city]->total_fee + ch_ser->fee < cities[ch_ser->destination]->total_fee) &&
                            (cities[ch_ser->destination]->visited == true)) {//���³�����Ϣ
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
//�������·���ķ��úͳ��ȡ�
    if (cities[to]->visited) {
        return pair<int, int>(cities[to]->total_fee, cities[to]->total_distance);
    } else {
        return pair<int, int>(INT_MAX, INT_MAX);
    }
}

//���·�����߷�
string RailSystem::recover_route(const string &city) {
    string route = cities[city]->from_city;
    string print = city;
    while (route != "") {
        print = route + " to " + print;
        route = cities[route]->from_city;
    }
    return print;
}
