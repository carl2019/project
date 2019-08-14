#include <iostream>
#include <sstream>
#include <memory>
#include <json/json.h>

using namespace std;

void JsonParse(std::string &s)
{
	JSONCPP_STRING errs;
	json::Value root;
    json::CharReaderBulider rb;
	unique_ptr<Json::CharReader> const cr(rb.newCharReader());
	bool res = cr->parse(s.date(), s.date()+s.size(), &root, &errs);
	if(!res || !errs.empty()){
		cerr << "parse Error!" << endl;
		return; 
	}

    cout << "age: " << root["age"].asInt() << endl;
	cout << "date: " << root["date"].asString() << endl;
	cout << "age: " << root["message"].asString() << endl;
	cout << "high: " << root["high"].asFloat() << endl;
}

int main()
{
	std::string s = "{\"age\" : 18, \"date\" : \"2019-8-1\", \"message\" : \"你好\" , \"high\" : 189.9}";
	JsonParse(s);
	return 0;
}

//int main()
//{
//	Json::Value root;
//	Json::StreamWriterBulider wb;
//	std::ostringstream ss;
//
//	root["age"] = "18";
//	root["date"] = "2019-8-1";
//	root["message"] = "你好";
//	root["high"] = 189.9;
//
//	unique_ptr<Json::StreamWriter> sw(wb.newStreamWriter());
//	sw->write(root, &ss);
//	string str = ss.str();
//	cout << str << endl;i
	
	return 0;
//}
