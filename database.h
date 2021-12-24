#pragma once
#include<string>
#include<vector>
#include<sstream>
#include<array>
#include<map>
#include<fstream>
#include<iostream>

#ifndef DATABASE_H_
#define DATABASE_H_

class Database {
/*   È«¶¨³¤£¡£¡£¡£¡£¡£¡£¡£¡£¡
###### pos_map_pair.first
0	V
1	SIA	
2	Be
3	He

###### pos_map_pair.second
0	F_mig
1	E_mig
2	F1
3	E_bind1
4	E_mig1
5	F2
6	E_bind2
7	E_mig2
8	F3
9	E_bind3
10	E_mig3
11	F4
12	E_bind4
13	E_mig4
14	radius
15	site
16	F_rot
17	E_rot
18	F_tm
19	E_tm
*/
public:
	Database(std::string _posb);
	std::map<std::array<int, 4>, std::array<double, 20>> pos_map;
};	


#endif 