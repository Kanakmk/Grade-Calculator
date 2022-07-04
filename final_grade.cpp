#include <iostream>
#include <stdexcept>
#include <string>
#include <map>
#include <set>
#include <fstream>
#include <numeric>
#include <algorithm>
#include <vector>
#include <sstream>
#include <utility>
#include <iterator>

#include "final_grade.h"

// type declarations
using StudentInfo = std::map<std::string, std::string>;
using IDInfo = std::map<std::string, std::map<std::string, std::string>>;
using AssignmentPair = std::pair<std::string,std::string>;

// generic function to get scores based on the category provided
std::vector<int> GetScores(StudentInfo assignments, std::string category) {
  // erases all instances of scores not in the category
  std::erase_if(assignments, [category] (const auto& p) {
    auto const& [name, score] = p; return !(name.find(category)!=std::string::npos);
  });
  std::vector<std::string> strpoints; // string vector of scores
  std::transform(assignments.begin(), assignments.end(), std::back_inserter(strpoints), [](auto &kv){
    return kv.second;
  });
  std::vector<int> intpoints; // int version of scores
  std::transform(strpoints.begin(), strpoints.end(), std::back_inserter(intpoints), [](const std::string& str){
    try{return std::stoi(str);}catch(std::invalid_argument& e){return 0;}
  }); // returns 0 if string is blank or not an int
  return intpoints;
}

int GetPointTotalForStudent(const StudentInfo & stuinfo, const std::string & category){
  StudentInfo assignments = stuinfo;
  std::vector<int> intpoints = GetScores(assignments, category); // get scores according to category
  return std::accumulate(intpoints.begin(), intpoints.end(), 0); // sum of all scores
}

int GetTopNHomeworkTotalForStudent(const StudentInfo & stuinfo, const int & tally){
  StudentInfo assignments = stuinfo;
  std::vector<int> intpoints = GetScores(assignments, "HW"); // gets all scores in HW category
  std::sort(intpoints.begin(), intpoints.end(), std::greater<int>()); // sorts in descending order
  if(tally < static_cast<int>(intpoints.size())) // only adds up top HWs if tally doesn't exceed number of scores
    return std::accumulate(intpoints.begin(), intpoints.begin() + tally, 0);
  return std::accumulate(intpoints.begin(), intpoints.end(), 0); // else it tallies up everything
}

int GetNumberOfMissingLabsForStudent(const StudentInfo & stuinfo){
  StudentInfo assignments = stuinfo;
  std::vector<int> intpoints = GetScores(assignments, "Lab"); // gets all labs scores
  // counts how many scores aren't equal to exactly 1
  int count = static_cast<int>(std::count_if(intpoints.begin(), intpoints.end(), [] (int x) {return x!=1;}));
  return count;
}

int GetPointTotalForStudent(const StudentInfo & stuinfo){
  StudentInfo assignments = stuinfo;
  std::vector<int> intpoints = GetScores(assignments, "Project"); 
  int total = std::accumulate(intpoints.begin(), intpoints.end(), 0); // sums project scores
  intpoints = GetScores(assignments, "Exam");
  total += std::accumulate(intpoints.begin(), intpoints.end(), 0); // sums exam scores
  total += GetTopNHomeworkTotalForStudent(stuinfo, 15); // sums top 15 homework scores to account f or 2 drops
  intpoints = GetScores(assignments, "Honors");
  total -= std::accumulate(intpoints.begin(), intpoints.end(), 0); // subtracts honors project score if avaliable
  return total;
}

// generic split function from 
// https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
std::vector<std::string> split(const std::string & line, char delim) {
  std::vector<std::string> result;
  std::stringstream ss(line);
  std::string item;
  while (std::getline(ss, item, delim)) {result.push_back(item);}
  return result;
}

IDInfo GetIDToInfoFromCSV(const std::string & filename){
  IDInfo idstuinfo;
  std::vector<std::string> keys;
  std::ifstream input(filename); // opens file f or reading
  std::string line;
  std::getline(input, line); // gets assignment names vector
  keys = split(line, ','); // splits by commas
  while (input.good()) {
    StudentInfo stuinfo;
    std::getline(input, line); // iterates through each line
    std::vector<std::string> values = split(line, ','); // splits each line
    if(values.size() != 40) {values.push_back("");} // adds empty string to make all vectors the same size
    // combines the assignment names vector and points vector into a pair 
    // https://stackoverflow.com/questions/4946424/creating-a-map-from-two-vectors
    std::transform(keys.begin(), keys.end(), values.begin(), std::inserter(stuinfo, stuinfo.end()), [](auto a, auto b){
        return std::make_pair(a, b);
    });
    idstuinfo.insert({stuinfo.at("ID"), stuinfo}); // inserts pair into map
  }
  input.close();
  return idstuinfo;
}

// calculates the gradepoint based on the number of points earned
double CalculateGradePoint(const int & total) {
  double gradepoint = 0.0;
  if(total >= 900) gradepoint = 4.0;
  else if(total < 900 && total >= 850) gradepoint = 3.5;
  else if(total < 850 && total >= 800) gradepoint = 3.0;
  else if(total < 800 && total >= 750) gradepoint = 2.5;
  else if(total < 750 && total >= 700) gradepoint = 2.0;
  else if(total < 700 && total >= 650) gradepoint = 1.5;
  else if(total < 650 && total >= 600) gradepoint = 1.0;
  return gradepoint;
}

std::map<std::string, double> GetIDToGrade(const IDInfo & idstudinfo){
  std::map<std::string, double> grades;
  // calculates gradepoint and lab penalty if it exists
  std::transform(idstudinfo.begin(), idstudinfo.end(), std::inserter(grades, grades.end()), [] (const auto & p) {
    auto const& [id, scores] = p;
    int total = GetPointTotalForStudent(scores);
    double gradepoint = CalculateGradePoint(total);
    int labs = GetNumberOfMissingLabsForStudent(scores)-2; // allowed to miss 2 labs
    if(labs > 0) {double reduction = labs * 0.5; gradepoint = gradepoint - reduction;} // -0.5 per lab missed after 2 drops
    if(gradepoint < 1.0) {gradepoint = 0.0;} // if gradepoint is less than 1 default grade is 0
    return std::make_pair(id, gradepoint);}); // pairs the ID of student with their respective gradepoint
  return grades;
}

std::map<std::pair<std::string, double>, int> CombineStudentInfo(const IDInfo & idstudinfo) {
  std::map<std::string, double> grades = GetIDToGrade(idstudinfo); // gets all grades f or students
  std::vector<int> honorsscores; // makes a vector containing all honors scores
  std::transform(idstudinfo.begin(), idstudinfo.end(), std::back_inserter(honorsscores), [] (const auto & p) {
    auto const& [id, scores] = p; int s = GetScores(scores, "Honors")[0]; return s;
  });
  std::map<std::pair<std::string, double>, int> eligible; // map of all students' IDs, gradepoints, and honors scores
  // combines all three things into a map
  std::transform(grades.begin(), grades.end(), honorsscores.begin(), std::inserter(eligible, eligible.end()), [](auto a, auto b){
    return std::make_pair(a, b);
  }); 
  return eligible;
}

std::set<std::string> GetStudentsEligibleForHonorsCredit(const IDInfo & idstudinfo, const int & mingrade){
  std::set<std::string> honors;
  std::map<std::pair<std::string, double>, int> eligible = CombineStudentInfo(idstudinfo);
  // iterates through map
  for(std::map<std::pair<std::string,double>,int>::iterator itr = eligible.begin(); itr != eligible.end(); itr++){
    // checks if honors score is greater than/equal to mingrade and if gradepoint is at least 3.5
    if( itr->second >= mingrade && (itr->first).second >= 3.5) 
      honors.insert((itr->first).first); // inserts student ID into set
  }
  return honors;
}
