#include"TextSimilarity.h"
#include "cppjieba/Jieba.hpp"
using namespace std;

//1、首先获取停用词
//2、对文件中的文章进行分词，计算出关键字的数量，并保存在
//3、对关键字按照数目进行排序
//4、按照顺序选出两个文件中的候选词
//5、用候选词的数目制作向量表
//6、根据余弦相似度的公式计算出相似度


string TextSimilarity::GBKToUTF8(string p)
{
	int len = MultiByteToWideChar(CP_ACP, 0, p.c_str(), -1, NULL, 0);
	wchar_t *uft16 = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, p.c_str(), -1, uft16, len);
	len = WideCharToMultiByte(CP_UTF8, 0, uft16, -1, NULL, 0, NULL, NULL);
	char *a = new char[len];
	WideCharToMultiByte(CP_UTF8, 0, uft16, -1, a, len, NULL, NULL);
	string str(a);
	return str;
}

string TextSimilarity::UTF8ToGBK(string p)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, p.c_str(), -1, NULL, 0);
	wchar_t *uft16 = new wchar_t[len];
	MultiByteToWideChar(CP_UTF8, 0, p.c_str(), -1, uft16, len);
	len = WideCharToMultiByte(CP_ACP, 0, uft16, -1, NULL, 0, NULL, NULL);
	char *a = new char[len];
	WideCharToMultiByte(CP_ACP, 0, uft16, -1, a, len, NULL, NULL);
	string str(a);
	return str;
}

//获取停用词
//1、打开文件，获取文件中的每一行的停用词
//2、将停用词保存在set中
void TextSimilarity::getStopWordTable(const char* stopWordFile)
{
	ifstream fin(stopWordFile);
	if (!fin.is_open())
	{
		cout << "open file2 error" << endl;
		return;
	}
	string ptr;
	while (!fin.eof())
	{
		//因为文件中没有停用词占一行，所以读取一个停用词
		getline(fin, ptr);
		//将读取的停用词保存在set中
		_stopWordSet.insert(ptr);
	}
	fin.close();
}
//构造函数
TextSimilarity::TextSimilarity(string dict)
	:DICT(dict)
	, DICT_PATH(dict + "/jieba.dict.utf8")
	, HMM_PATH(dict + "/hmm_model.utf8")
	, USER_DICT_PATH(dict + "/user.dict.utf8")
	, IDF_PATH(dict + "/idf.utf8")
	, STOP_WORD_PATH(dict + "/stop_words.utf8")
	, _jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH)
	, _maxWordNumber(10)
{
	//初始化停用词
	getStopWordTable(STOP_WORD_PATH.c_str());
}
//比较函数
bool cmpREserve(pair<string, int>lp, pair<string, int>rp)
{
	return lp.second > rp.second;
}

//对文件中的关键字按照数目进行排序
vector<std::pair<std::string, int>> TextSimilarity::sortByValueReverse(TextSimilarity::wordFreq& wf)
{
	//因为sort只能对序列式的数组进行排序，所以将map中的数据保存在二维数组中进行排序
	vector<std::pair<std::string, int>> wfvector(wf.begin(), wf.end());
	sort(wfvector.begin(), wfvector.end(), cmpREserve);
	return wfvector;
}
//分词、计算出词频
TextSimilarity::wordFreq TextSimilarity::getWordFreq(const char *file)
{
	ifstream fin(file);
	if (!fin.is_open())
	{
		cout << "open file1 error" << endl;
		return wordFreq();
	}
	string line;
	wordFreq wf;
	while (!fin.eof())
	{
		getline(fin, line);
		//首先将文件的格式转换为哦UTF8的格式，然后进行分词
		line = GBKToUTF8(line);
		vector<string> words;
		//分词、将关键字保存在word文档中
		_jieba.Cut(line, words, true);
		for (const auto & e : words)
		{
			//如果是当前词是停用词的话，直接跳过
			if (_stopWordSet.count(e))
				continue;
			else
			{
				//如果这个关键字已经保存的话，将他的数目++
				if (wf.count(e))
					wf[e]++;
				//如果这个关键字是第一次出现的话，将他的数目设置成1
				else
					wf[e] = 1;
			}
		}
	}
	return wf;
}

//选择候选词
void TextSimilarity::selectAimWords(std::vector<std::pair<std::string, int>>& wfvec, TextSimilarity::wordSet& wset)
{
	int len = wfvec.size();
	int num = _maxWordNumber;
	//如果这个文件中的关键字的数量小于候选词的规定数目的时候，此时将按照关键字的数目来选择候选词
	if (_maxWordNumber > len)
		num = len;
	//将候选词的名字保存在set中
	for (int i = 0; i < num; i++)
	{
		wset.insert(wfvec[i].first);
	}
}

//获取向量表
vector<double> TextSimilarity::getOneHot(TextSimilarity::wordSet& wset, TextSimilarity::wordFreq& wf)
{
	vector<double> str;
	//根据候选词的来将候选词的数目保存在向量表中
	for (auto & e : wset)
	{
		if (wf.count(e))
		{
			str.push_back(wf[e]);
		}
		else
		{
			str.push_back(0);
		}
	}
	return str;
}

//计算相似度
double TextSimilarity::cosine(std::vector<double> oneHot1, std::vector<double> oneHot2)
{
	double ptr = 0.0;
	double deniminator = 0;
	double deniminator2 = 0;
	for (int i = 0; i < oneHot1.size(); i++)
	{
		ptr += (oneHot1[i] * oneHot2[i]);
	}
	for (int i = 0; i < oneHot1.size(); i++)
	{
		deniminator += (oneHot1[i] * oneHot1[i]);
	}
	for (int i = 0; i < oneHot1.size(); i++)
	{
		deniminator2 += (oneHot2[i] * oneHot2[i]);
	}
	//cout << ptr <<" "<< sqrt(deniminator) * sqrt(deniminator2) << endl;
	return (ptr / (sqrt(deniminator) * sqrt(deniminator2)));
}


double TextSimilarity::getTextSimilarity(const char* file1, const char* file2)
{
	TextSimilarity wf("dict");
	std::unordered_map<std::string, int> ptr = wf.getWordFreq(file1);//获取文章1中的词语及其个数
	std::unordered_map<std::string, int> ptr2 = wf.getWordFreq(file2);//获取文章2中的词语的及其个数
	std::vector<std::pair<std::string, int>> wfvec = wf.sortByValueReverse(ptr);//将词语按照数目进行排序
	std::vector<std::pair<std::string, int>> wfvec2 = wf.sortByValueReverse(ptr2);
	std::unordered_set<std::string> str;//用来获取文章1和文章2中的候选词
	wf.selectAimWords(wfvec, str);
	wf.selectAimWords(wfvec2, str);
	std::vector<double> oneHont = wf.getOneHot(str, ptr);//在文章1中找出str中每个词出现的次数
	std::vector<double> oneHont2 = wf.getOneHot(str, ptr2);//在文章2中找出str每个词出现的次数
	return wf.cosine(oneHont, oneHont2);//根据余弦相似度计算出相似度
}