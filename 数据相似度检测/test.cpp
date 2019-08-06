#include"TextSimilarity.h"
#include"FindFiles.h"
using namespace std;

void Select()
{
	cout << "**********************" << endl;
	cout << "***** 1.文件搜索 *****" << endl;
	cout << "***** 2.文本查重 *****" << endl;
	cout << "**********************" << endl;
}

int main()
{
	TextSimilarity wf("dict");
	//cout << "这两个文件的相似度为" << wf.getTextSimilarity("test.txt", "test2.txt") << endl;
	int n = 0;
	Select();
	cin >> n;
	switch (n)
	{
	case 1:
		Find();
		break;
	case 2:
		cout << "这两个文件的相似度为" << wf.getTextSimilarity("test.txt", "test2.txt") << endl;
		break;
	default:
		break;
	}
	return 0;
}
