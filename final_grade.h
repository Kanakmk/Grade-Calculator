#ifndef FINAL_GRADES
#define FINAL_GRADES

#include <iostream>
#include <string>
#include <map>
#include <set>

using StudentInfo = std::map<std::string, std::string>;
using IDInfo = std::map<std::string, std::map<std::string, std::string>>;

int GetPointTotalForStudent(const StudentInfo & stuinfo, const std::string & category);
int GetTopNHomeworkTotalForStudent(const StudentInfo & stuinfo, const int & tally);
int GetNumberOfMissingLabsForStudent(const StudentInfo & stuinfo);
int GetPointTotalForStudent(const StudentInfo & stuinfo);
IDInfo GetIDToInfoFromCSV(const std::string & filename);
std::map<std::string, double> GetIDToGrade(const IDInfo & id_to_info);
std::set<std::string> GetStudentsEligibleForHonorsCredit(const IDInfo & idstudinfo, const int & mingrade);

#endif
