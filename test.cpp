#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <unordered_set>

using namespace std;

int main() {
    map <string, vector<string> > subscriptions;

    // string s1 = "upb";
    // string s2 = "fmi";
    // cout << s1 + "\n";

    // vector<string> v1;
    // v1.push_back("C1");
    // v1.push_back("C2");

    // vector<string> v2;
    // v2.push_back("C3");
    // v2.push_back("C4");

    // cout << v1[1];
    // for (auto i = v1.begin(); i != v1.end(); ++i)
    //     cout << *i << " ";

    // cout << endl;

    // subscriptions.insert ( std::pair<string,vector<string>>(s1, v1) );
    // subscriptions.insert ( std::pair<string,vector<string>>(s2, v2) );

    // subscriptions.at("upb").push_back("C5");

    // vector<string> aux = subscriptions.at(s1);

    // for (auto i = aux.begin(); i != aux.end(); ++i)
    //     cout << *i << " ";
    // cout << endl;

    // // if(subscriptions.at("utcb") == NULL) {
    // //     printf("%d\n", 15);
    // // }
    // cout << subscriptions.count("upb") << endl;

    // map <string, unordered_set<string> > subs;  
    map<string, int> sockets;
    sockets["C1"] = 5;
    cout << sockets.at("C1") << endl;

    sockets["C1"] = 6;
    cout << sockets.at("C1") << endl;
    

}