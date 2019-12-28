#include<bits/stdc++.h>
#define endl '\n'
using namespace std;

void rev(string tar){
	for(int i=tar.length()-1; i>=0; i--)
		cout << tar[i];
	cout << endl;
}
void spl(string tar, string sp){
	char *token = strtok(&tar[0], &sp[0]);
	while (token != NULL){
		cout << token << " ";
		token = strtok(NULL, &sp[0]);
	}
	cout << endl;
}
int main(int argc, char *argv[]){
	if(argc < 3){
		cout << "jizz " << endl;
		exit(0);
	}
	string in = "-------------------Input file ";
	string out = "-------------------End of input file ";
	string user = "*******************User input*****************************";
	cout << in << argv[1];
	for(int i=in.length()+strlen(argv[1]); i<user.length(); i++)
		cout << "-";
	cout << endl;

	ifstream file(argv[1]);
	string com, tar;
	while(file >> com >> tar){		
		if(com == "reverse"){
				cout << "reverse " << tar << endl;
				rev(tar);
			}
		else if(com == "split"){
			cout << "split " << tar << endl;
			spl(tar, argv[2]);
		}
	}
	file.close();

	cout << out << argv[1];
	for(int i=out.length()+strlen(argv[1]); i<user.length(); i++)
		cout << "-";
	cout << endl << user << endl;

	while(cin >> com){
		if(com == "exit")
			exit(0);
		else if(com == "reverse" || com == "split"){
			cin >> tar;
			if(com == "reverse") rev(tar);
			else spl(tar, argv[2]);
		}
		else
			cout << "jizz" << endl;
	}
}
