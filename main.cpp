#include <memory.h>
#include <stdlib.h>

#include <algorithm>
#include <sstream>
#include <iostream>
#include <utility>
#include <vector>
#include <fstream>
#include <string>
#include <cerrno>
#include <iomanip>

using namespace std;
string prefix = "##";
string suffix = "##";


class util {
public:
	static std::string get_file_contents(const char *filename)
	{
		std::ifstream in(filename, std::ios::in | std::ios::binary);
		if (in)
		{
			std::string contents;
			in.seekg(0, std::ios::end);
			contents.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());
			in.close();
			return(contents);
		}
		throw(errno);
	}
};

using namespace std;

class float_seq {
public:
	float fseq;
	int iseq;
	size_t index_start;
	size_t index_end;
};

string getFloatString(const char* buf)
{
	bool flag_dot = false;
	int i=0;
	for(i;*(buf+i) != '\0';++i){
		if(*(buf+i) < '0' || *(buf+i) > '9'){
			if(*(buf+i) == '.' && i > 0 && !flag_dot){
				flag_dot = true;
			}else{
				break;
			}
		}
	}
	return string(buf, i);
}

vector<float_seq*> check_fs(const string& buf)
{
	vector<float_seq*> ret;
	for(int i=0;i<buf.size()-prefix.size()-suffix.size()-1;++i){
		if(memcmp(prefix.c_str(), buf.c_str()+i, prefix.length()) == 0){
			string fs = getFloatString(buf.c_str()+i+prefix.length());
			if(fs.length() > 0 && memcmp(suffix.c_str(), buf.c_str()+i+prefix.length()+fs.length(), suffix.length()) == 0){
				float_seq* seq = new float_seq();
				seq->fseq = atof(fs.c_str());
				seq->iseq = -1;
				seq->index_start = i;
				seq->index_end = i+prefix.length()+fs.length()+suffix.length();
				ret.push_back(seq);
				i += prefix.length()+fs.length()+suffix.length()-1;
			}
		}
	}
	return ret;
}

bool sort_index_start(const float_seq* s1, const float_seq* s2)
{
	return s1->index_start < s2->index_start;
}

int main(int argc, char** argv)
{
	if(argc < 3){
		printf("usage: %s <prefix> <suffix> <path_file_in> <path_file_out> [num_padding] [chara_padding]\n", *(argv+0));
		return 1;
	}
	prefix = string(*(argv+1));
	suffix = string(*(argv+2));
	string path_file(*(argv+3));
	string path_file_out(*(argv+4));
	string buf;
	int num_padding = 0;
	char chara_padding = '0';
	if(argc >= 6) num_padding = atoi(*(argv+5));
	if(argc >= 7) chara_padding = *(*(argv+6)+0);
	try{
		buf  = util::get_file_contents(path_file.c_str());
	}catch(int e){
		cout << "file open error:" << path_file << endl;
		return 1;
	}

	vector<float_seq*> fs = check_fs(buf);

	int seq = 1;
	while(true){
		float_seq* min = NULL;
		float f_min = 999999;
		for(vector<float_seq*>::iterator ite = fs.begin();ite != fs.end();++ite){
			if((*ite)->iseq == -1 && (*ite)->fseq < f_min){
				f_min = (*ite)->fseq;
				min = *ite;
			}
		}
		if(min){
			min->iseq = seq;
		}else{
			break;
		}
		++seq;
	}

	for(vector<float_seq*>::iterator ite = fs.begin();ite != fs.end();++ite){
		cout << (*ite)->fseq << " -> " << (*ite)->iseq << endl;
		cout << "index(" << (*ite)->index_start << ", " << (*ite)->index_end << ")" << endl;
	}

	//std::sort(fs.begin(), fs.end(), sort_index_start);

	ofstream f_out((path_file_out).c_str());
	if(!f_out.fail()){
		int index = 0;
		for(vector<float_seq*>::iterator ite = fs.begin();ite != fs.end();++ite){
			f_out.write(buf.c_str()+index, (*ite)->index_start-index);
			stringstream ss;
			f_out << prefix;
			if(num_padding > 0){
				f_out << std::setw(num_padding) << std::setfill(chara_padding);
			}
			f_out << (*ite)->iseq;
			f_out << suffix;
			index = (*ite)->index_end;
		}
		f_out.write(buf.c_str()+index, buf.length()-index);
		f_out.close();
	}

	for(vector<float_seq*>::iterator ite = fs.begin();ite != fs.end();++ite){
		delete(*ite);
	}

	return 0;
}
