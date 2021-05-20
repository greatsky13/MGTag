#include <stdio.h>
#include <string>
#include "Matrixmap.h"
#include "MatrixMapOp.h"
#include <map>
using namespace std;

int main(int argc, char **argv)
{
	map<char,Node> map1;
	MatrixMapOp::matrixFactorization(21,map1);
	map<char,Node>::iterator mapiter = map1.begin();
	for(; mapiter != map1.end() ; mapiter++){
		printf("%d  %d  %d\n",mapiter->first,mapiter->second.left, mapiter->second.right);
	}
   return 0;
}
