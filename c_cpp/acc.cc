#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

int main(){

  fstream file1, file2;
  file1.open( "result1.txt"        , std::fstream::in );
  file2.open( "testing_answer.txt" , std::fstream::in );
  char buf1[200];
  char buf2[200];
  int  counter = 0;

  for( int i = 0; i < 2500; ++i ){
    file1.getline( buf1, 200, '\n' );
    file2.getline( buf2, 200, '\n' );
    if( buf1[7] != buf2[7] ){
      cout << buf1 << "\t" << buf2 << endl;
      counter ++;
    }
  }

  file1.close();
  file2.close();

  file1.open( "acc.txt" , std::fstream::trunc | std::fstream::out );
  file1 << (2500-counter)/(2500.0f) << endl;

  return 0;


}
