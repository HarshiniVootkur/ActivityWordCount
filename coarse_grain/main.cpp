#include <chrono>
#include <fstream>
#include <sstream>
#include <vector>

#include <thread>
#include <mutex>

#include "Dictionary.hpp"
#include "MyHashtable.hpp"

using namespace std;

//Tokenize a string into individual word, removing punctuation at the
//end of words
vector<vector<string>> tokenizeLyrics(const vector<string> files) {
  vector<vector<string>> ret;

  for(auto filename : files) {
    //cout<<"reading "<<filename<<"\n";
    vector<string> my_vect;
    ifstream in (filename);

    string line;

    //For each line
    while (getline(in, line, '\n')) {
      //Skip all like starting with [
      if (line[0] == '[')
        continue;

      stringstream ssline (line);
      //For all words
      while (ssline) {
        string word;
        ssline >> word;
        if (ssline) {
          //remove punctuation at the end of word
          while (word.length() > 0
                 && ispunct(word[word.length() - 1])) {
            word.pop_back();
          }
          my_vect.push_back(word);
        }
      }
    }
    //cout<<"read "<<my_vect.size()<<" words\n";
    ret.push_back(my_vect);
  }
  return ret;
}

//Populate hash table
void hash_populate(vector<string> filecontent, Dictionary<string,int>& dict,mutex& mut)
{
  for (auto & w : filecontent)
    {
      //lock the mutex to avoid race condition
      mut.lock();
      
      int count=dict.get(w);
      ++count;
      dict.set(w,count);
      
      //unlock mutex
      mut.unlock();
    }
}

int main(int argc, char **argv)
{
  if (argc < 4) {
    cerr<<"usage: ./main <sources> <testword> <threshold>"<<std::endl;
    return -1;
  }

  // Parse Command Line
  string source = argv[1];
  string testWord = argv[2];
  int32_t thresholdCount = stoi(argv[3]);

  // Obtain List of Files
  vector<string> files;
  ifstream in (source);
  string line;
  while (getline(in, line, '\n')) {
    files.push_back(line);
  }

  // Tokenize Lyrics
  auto wordmap = tokenizeLyrics(files);

  MyHashtable<string, int> ht;
  Dictionary<string, int>& dict = ht;
  
  // write code here

  mutex m;
  vector<thread> mythread;
  auto startTime=chrono::steady_clock::now();
  for(int i=0;i<wordmap.size();i++)
    {
      mythread.push_back(thread(hash_populate,wordmap.at(i),ref(ht),ref(m)));
    }

  for(auto &t: mythread)
    {
      t.join();
    }

  auto stopTime=chrono::steady_clock::now();
  chrono::duration<double> time_elapsed=stopTime-startTime;

  // Check Hash Table Values 
  /* (you can uncomment, but this must be commented out for tests)
  for (auto it : dict) {
    if (it.second > thresholdCount)
      cout << it.first << " " << it.second << endl;
  }
  */
  // Do not touch this, need for test cases
  cout << ht.get(testWord) << endl;
  cerr<<time_elapsed.count()<<"\n";

  return 0;
}
